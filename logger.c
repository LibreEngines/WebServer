#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

static FILE *log_file_handle = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static log_level_t min_log_level = LOG_LEVEL_INFO;

int logger_init(const char *log_file) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file_handle) {
        fclose(log_file_handle);
    }
    
    if (log_file) {
        log_file_handle = fopen(log_file, "a");
        if (!log_file_handle) {
            pthread_mutex_unlock(&log_mutex);
            return -1;
        }
    } else {
        log_file_handle = stdout;
    }
    
    pthread_mutex_unlock(&log_mutex);
    
    log_info("Logger initialized");
    return 0;
}

void logger_cleanup(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file_handle && log_file_handle != stdout && log_file_handle != stderr) {
        fclose(log_file_handle);
        log_file_handle = NULL;
    }
    
    pthread_mutex_unlock(&log_mutex);
}

void log_message(log_level_t level, const char *format, ...) {
    if (level < min_log_level) {
        return;
    }
    
    pthread_mutex_lock(&log_mutex);
    
    if (!log_file_handle) {
        log_file_handle = stdout;
    }
    
    char *timestamp = get_timestamp();
    const char *level_str = log_level_string(level);
    
    // Print timestamp and log level
    fprintf(log_file_handle, "[%s] %s: ", timestamp, level_str);
    
    // Print the actual message
    va_list args;
    va_start(args, format);
    vfprintf(log_file_handle, format, args);
    va_end(args);
    
    fprintf(log_file_handle, "\n");
    fflush(log_file_handle);
    
    // Also print to stdout for important messages
    if (level >= LOG_LEVEL_WARNING && log_file_handle != stdout) {
        fprintf(stdout, "[%s] %s: ", timestamp, level_str);
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    
    free(timestamp);
    pthread_mutex_unlock(&log_mutex);
}

void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_mutex);
    
    if (LOG_LEVEL_DEBUG >= min_log_level && log_file_handle) {
        char *timestamp = get_timestamp();
        fprintf(log_file_handle, "[%s] DEBUG: ", timestamp);
        vfprintf(log_file_handle, format, args);
        fprintf(log_file_handle, "\n");
        fflush(log_file_handle);
        free(timestamp);
    }
    
    pthread_mutex_unlock(&log_mutex);
    va_end(args);
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_mutex);
    
    if (LOG_LEVEL_INFO >= min_log_level && log_file_handle) {
        char *timestamp = get_timestamp();
        fprintf(log_file_handle, "[%s] INFO: ", timestamp);
        vfprintf(log_file_handle, format, args);
        fprintf(log_file_handle, "\n");
        fflush(log_file_handle);
        free(timestamp);
    }
    
    pthread_mutex_unlock(&log_mutex);
    va_end(args);
}

void log_warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_mutex);
    
    if (log_file_handle) {
        char *timestamp = get_timestamp();
        fprintf(log_file_handle, "[%s] WARNING: ", timestamp);
        vfprintf(log_file_handle, format, args);
        fprintf(log_file_handle, "\n");
        fflush(log_file_handle);
        
        // Also print to stdout
        if (log_file_handle != stdout) {
            fprintf(stdout, "[%s] WARNING: ", timestamp);
            va_start(args, format);
            vfprintf(stdout, format, args);
            va_end(args);
            fprintf(stdout, "\n");
            fflush(stdout);
        }
        
        free(timestamp);
    }
    
    pthread_mutex_unlock(&log_mutex);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_mutex);
    
    if (log_file_handle) {
        char *timestamp = get_timestamp();
        fprintf(log_file_handle, "[%s] ERROR: ", timestamp);
        vfprintf(log_file_handle, format, args);
        fprintf(log_file_handle, "\n");
        fflush(log_file_handle);
        
        // Also print to stderr
        fprintf(stderr, "[%s] ERROR: ", timestamp);
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        fflush(stderr);
        
        free(timestamp);
    }
    
    pthread_mutex_unlock(&log_mutex);
    va_end(args);
}

const char *log_level_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

char *get_timestamp(void) {
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    
    char *timestamp = malloc(32);
    if (timestamp) {
        strftime(timestamp, 32, "%Y-%m-%d %H:%M:%S", local_time);
    }
    
    return timestamp;
}

