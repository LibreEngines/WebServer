#include "ip_manager.h"
#include "logger.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

// Global IP context
static ip_context_t *global_ip_ctx = NULL;

int ip_init(ip_context_t *ip_ctx) {
    if (!ip_ctx) return -1;
    
    memset(ip_ctx, 0, sizeof(ip_context_t));
    ip_ctx->pool_count = 0;
    
    global_ip_ctx = ip_ctx;
    
    // Load existing IPs if file exists
    ip_load_ips(ip_ctx, "ips.dat");
    
    log_info("IP management system initialized with %d pools", ip_ctx->pool_count);
    return 0;
}

void ip_cleanup(ip_context_t *ip_ctx) {
    if (!ip_ctx) return;
    
    // Save IPs before cleanup
    ip_save_ips(ip_ctx, "ips.dat");
    
    log_info("IP management system cleaned up");
}

int ip_create_pool(ip_context_t *ip_ctx, int pool_number, const char *pool_name, 
                  const char *base_ip, const char *netmask) {
    if (!ip_ctx || !pool_name || !base_ip || !netmask) return -1;
    
    // Validate pool number
    if (pool_number <= 0 || pool_number > MAX_POOLS) {
        log_warning("Invalid pool number: %d", pool_number);
        return -2;
    }
    
    // Check if pool already exists
    if (ip_get_pool(ip_ctx, pool_number) != NULL) {
        log_warning("Pool already exists: %d", pool_number);
        return -3;
    }
    
    // Check if we have space for new pool
    if (ip_ctx->pool_count >= MAX_POOLS) {
        log_error("Maximum pools reached");
        return -4;
    }
    
    // Create new pool
    pool_t *pool = &ip_ctx->pools[ip_ctx->pool_count];
    pool->pool_number = pool_number;
    strncpy(pool->pool_name, pool_name, POOL_NAME_LENGTH - 1);
    
    // Convert and store base IP and netmask
    if (inet_pton(AF_INET, base_ip, &pool->base_ip) != 1) {
        log_warning("Invalid base IP address: %s", base_ip);
        return -5;
    }
    
    if (inet_pton(AF_INET, netmask, &pool->netmask) != 1) {
        log_warning("Invalid netmask: %s", netmask);
        return -6;
    }
    
    // Initialize all IPs in the pool as available
    for (int i = 0; i < MAX_ADDRESSES_POOL; i++) {
        pool->addresses[i].ip_address = htonl(ntohl(pool->base_ip) + i);
        pool->addresses[i].is_used = false;
        pool->addresses[i].pool_number = pool_number;
        memset(pool->addresses[i].allocated_to, 0, sizeof(pool->addresses[i].allocated_to));
        pool->addresses[i].allocation_time = 0;
    }
    
    // First and last addresses are reserved
    pool->addresses[0].is_used = true;
    pool->addresses[MAX_ADDRESSES_POOL-1].is_used = true;
    
    pool->available_ips = MAX_ADDRESSES_POOL - 2; // Subtract 2 for network and broadcast
    pool->used_ips = 2;
    pool->is_full = false;
    
    ip_ctx->pool_count++;
    
    // Immediately save to disk
    if (ip_save_ips(ip_ctx, "ips.dat") != 0) {
        log_error("Failed to save IP pool to disk");
        ip_ctx->pool_count--;
        return -7;
    }
    
    log_info("Pool created: %s (Number: %d)", pool_name, pool_number);
    return 0;
}

unsigned int ip_allocate_ip(ip_context_t *ip_ctx, int pool_number, const char *allocated_to) {
    if (!ip_ctx || !allocated_to) return 0;
    
    pool_t *pool = ip_get_pool(ip_ctx, pool_number);
    if (!pool) {
        log_warning("Pool not found: %d", pool_number);
        return 0;
    }
    
    if (pool->is_full) {
        log_warning("Pool %d is full", pool_number);
        return 0;
    }
    
    // Find first available IP
    for (int i = 1; i < MAX_ADDRESSES_POOL-1; i++) { // Skip first and last
        if (!pool->addresses[i].is_used) {
            pool->addresses[i].is_used = true;
            strncpy(pool->addresses[i].allocated_to, allocated_to, 
                   sizeof(pool->addresses[i].allocated_to) - 1);
            pool->addresses[i].allocation_time = time(NULL);
            
            pool->available_ips--;
            pool->used_ips++;
            
            if (pool->available_ips == 0) {
                pool->is_full = true;
            }
            
            // Immediately save to disk
            if (ip_save_ips(ip_ctx, "ips.dat") != 0) {
                log_error("Failed to save IP allocation to disk");
                // Roll back
                pool->addresses[i].is_used = false;
                pool->available_ips++;
                pool->used_ips--;
                return 0;
            }
            
            log_info("IP %s allocated to %s in pool %d", 
                    ip_to_string(pool->addresses[i].ip_address),
                    allocated_to, pool_number);
            
            return pool->addresses[i].ip_address;
        }
    }
    
    return 0;
}

int ip_release_ip(ip_context_t *ip_ctx, unsigned int ip_address) {
    if (!ip_ctx) return -1;
    
    for (int p = 0; p < ip_ctx->pool_count; p++) {
        pool_t *pool = &ip_ctx->pools[p];
        for (int i = 0; i < MAX_ADDRESSES_POOL; i++) {
            if (pool->addresses[i].ip_address == ip_address && pool->addresses[i].is_used) {
                pool->addresses[i].is_used = false;
                memset(pool->addresses[i].allocated_to, 0, sizeof(pool->addresses[i].allocated_to));
                pool->addresses[i].allocation_time = 0;
                
                pool->available_ips++;
                pool->used_ips--;
                pool->is_full = false;
                
                // Immediately save to disk
                if (ip_save_ips(ip_ctx, "ips.dat") != 0) {
                    log_error("Failed to save IP release to disk");
                    // Roll back
                    pool->addresses[i].is_used = true;
                    pool->available_ips--;
                    pool->used_ips++;
                    return -2;
                }
                
                log_info("IP %s released from pool %d", 
                        ip_to_string(ip_address), pool->pool_number);
                return 0;
            }
        }
    }
    
    log_warning("IP not found or already free: %s", ip_to_string(ip_address));
    return -3;
}

pool_t* ip_get_pool(ip_context_t *ip_ctx, int pool_number) {
    if (!ip_ctx) return NULL;
    
    for (int i = 0; i < ip_ctx->pool_count; i++) {
        if (ip_ctx->pools[i].pool_number == pool_number) {
            return &ip_ctx->pools[i];
        }
    }
    
    return NULL;
}

ip_t* ip_get_ip(ip_context_t *ip_ctx, unsigned int ip_address) {
    if (!ip_ctx) return NULL;
    
    for (int p = 0; p < ip_ctx->pool_count; p++) {
        for (int i = 0; i < MAX_ADDRESSES_POOL; i++) {
            if (ip_ctx->pools[p].addresses[i].ip_address == ip_address) {
                return &ip_ctx->pools[p].addresses[i];
            }
        }
    }
    
    return NULL;
}

int ip_save_ips(ip_context_t *ip_ctx, const char *filename) {
    if (!ip_ctx || !filename) return -1;
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        log_error("Failed to open IP file for writing: %s", filename);
        return -1;
    }
    
    // Write pool count
    fwrite(&ip_ctx->pool_count, sizeof(int), 1, file);
    
    // Write each pool
    for (int i = 0; i < ip_ctx->pool_count; i++) {
        // Write pool metadata
        fwrite(&ip_ctx->pools[i], sizeof(pool_t), 1, file);
    }
    
    fclose(file);
    log_info("Saved %d IP pools to %s", ip_ctx->pool_count, filename);
    return 0;
}

int ip_load_ips(ip_context_t *ip_ctx, const char *filename) {
    if (!ip_ctx || !filename) return -1;
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        log_info("IP file not found, starting with empty database");
        return 0;
    }
    
    // Read pool count
    if (fread(&ip_ctx->pool_count, sizeof(int), 1, file) != 1) {
        log_error("Failed to read pool count from file");
        fclose(file);
        return -1;
    }
    
    // Validate pool count
    if (ip_ctx->pool_count > MAX_POOLS) {
        log_error("Invalid pool count in file: %d", ip_ctx->pool_count);
        fclose(file);
        return -1;
    }
    
    // Read each pool
    for (int i = 0; i < ip_ctx->pool_count; i++) {
        if (fread(&ip_ctx->pools[i], sizeof(pool_t), 1, file) != 1) {
            log_error("Failed to read pool %d from file", i);
            fclose(file);
            return -1;
        }
    }
    
    fclose(file);
    log_info("Loaded %d IP pools from %s", ip_ctx->pool_count, filename);
    return 0;
}

char* ip_to_string(unsigned int ip_address) {
    static char str[INET_ADDRSTRLEN];
    struct in_addr addr;
    addr.s_addr = ip_address;
    inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
    return str;
}

unsigned int ip_string_to_ip(const char *ip_str) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str, &addr) != 1) {
        return 0;
    }
    return addr.s_addr;
}

void ip_cleanup_expired(ip_context_t *ip_ctx, time_t expiry_time) {
    if (!ip_ctx) return;
    
    time_t now = time(NULL);
    int expired_count = 0;
    
    for (int p = 0; p < ip_ctx->pool_count; p++) {
        pool_t *pool = &ip_ctx->pools[p];
        for (int i = 0; i < MAX_ADDRESSES_POOL; i++) {
            if (pool->addresses[i].is_used && 
                (now - pool->addresses[i].allocation_time) > expiry_time) {
                ip_release_ip(ip_ctx, pool->addresses[i].ip_address);
                expired_count++;
            }
        }
    }
    
    if (expired_count > 0) {
        log_info("Cleaned up %d expired IP allocations", expired_count);
    }
}