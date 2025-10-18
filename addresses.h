#ifndef IP_MANAGEMENT_H
#define IP_MANAGEMENT_H

#include <stdbool.h>
#include <time.h>

#define MAX_ADDRESSES_POOL 256
#define MAX_ADDRESSES_POOL_USE 254
#define MAX_POOLS 500
#define POOL_NAME_LENGTH 64

typedef struct {
    unsigned int ip_address;
    bool is_used;
    int pool_number;
    time_t allocation_time;
    char allocated_to[128];
} ip_t;

typedef struct {
    int pool_number;
    bool is_full;
    int available_ips;
    int used_ips;
    char pool_name[POOL_NAME_LENGTH];
    ip_t addresses[MAX_ADDRESSES_POOL];
} pool_t;

typedef struct {
    pool_t pools[MAX_POOLS];
    int pool_count;
} ip_context_t;

// Initialization and cleanup
int address_init(ip_context_t *address_ctx);
void address_cleanup(ip_context_t *address_ctx);

// IP Address Management
int address_register_ip(ip_context_t *address_ctx, unsigned int ip_address, int pool_number);
int address_release_ip(ip_context_t *address_ctx, unsigned int ip_address);
ip_t* address_get_ip(ip_context_t *address_ctx, unsigned int ip_address);

// Pool Management
int address_create_pool(ip_context_t *address_ctx, int pool_number, const char *pool_name);
int address_delete_pool(ip_context_t *address_ctx, int pool_number);
pool_t* address_get_pool(ip_context_t *address_ctx, int pool_number);

// Allocation Management
unsigned int address_allocate_ip(ip_context_t *address_ctx, int pool_number, const char *allocated_to);
int address_renew_allocation(ip_context_t *address_ctx, unsigned int ip_address, time_t new_expiry);

// Query Functions
int address_get_available_ips(ip_context_t *address_ctx, int pool_number);
int address_get_used_ips(ip_context_t *address_ctx, int pool_number);
bool address_is_ip_available(ip_context_t *address_ctx, unsigned int ip_address);

// Persistence
int address_save_ips(ip_context_t *address_ctx, const char *filename);
int address_load_ips(ip_context_t *address_ctx, const char *filename);

// Utility Functions
char* address_ip_to_string(unsigned int ip_address);
unsigned int address_string_to_ip(const char *ip_str);

#endif // IP_MANAGEMENT_H