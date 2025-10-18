#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

// String utilities
char *trim_whitespace(char *str);
char *url_decode(const char *str);
char *url_encode(const char *str);
int string_ends_with(const char *str, const char *suffix);
int string_starts_with(const char *str, const char *prefix);

// File utilities
int file_exists(const char *filename);
size_t get_file_size(const char *filename);
char *get_file_extension(const char *filename);

// Memory utilities
void safe_free(void **ptr);

// Security utilities
int is_safe_path(const char *path);
char *sanitize_filename(const char *filename);

// HTTP utilities
const char *get_mime_type(const char *filename);
char *get_status_message(int status_code);

// Forward declaration for accept_connections
void *accept_connections(void *arg);

#endif

