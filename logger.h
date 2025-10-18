#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} log_level_t;

// Logger initialization and cleanup
int logger_init(const char *log_file);
void logger_cleanup(void);

// Logging functions
void log_message(log_level_t level, const char *format, ...);
void log_debug(const char *format, ...);
void log_info(const char *format, ...);
void log_warning(const char *format, ...);
void log_error(const char *format, ...);

// Utility functions
const char *log_level_string(log_level_t level);
char *get_timestamp(void);

#endif

