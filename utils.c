#include "utils.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <strings.h>

char *trim_whitespace(char *str) {
    if (!str) return NULL;
    
    // Trim leading whitespace
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str; // All spaces
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = '\0';
    
    return str;
}

char *url_decode(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char *decoded = malloc(len + 1);
    if (!decoded) return NULL;
    
    size_t i = 0, j = 0;
    while (i < len) {
        if (str[i] == '%' && i + 2 < len) {
            // Decode percent-encoded character
            char hex[3] = {str[i+1], str[i+2], '\0'};
            decoded[j] = (char)strtol(hex, NULL, 16);
            i += 3;
        } else if (str[i] == '+') {
            // Convert '+' to space
            decoded[j] = ' ';
            i++;
        } else {
            decoded[j] = str[i];
            i++;
        }
        j++;
    }
    
    decoded[j] = '\0';
    return decoded;
}

char *url_encode(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1); // Worst case: all characters need encoding
    if (!encoded) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded[j++] = c;
        } else {
            sprintf(&encoded[j], "%%%02X", c);
            j += 3;
        }
    }
    
    encoded[j] = '\0';
    return encoded;
}

int string_ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return 0;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int string_starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) return 0;
    
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

int file_exists(const char *filename) {
    if (!filename) return 0;
    
    struct stat st;
    return stat(filename, &st) == 0;
}

size_t get_file_size(const char *filename) {
    if (!filename) return 0;
    
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

char *get_file_extension(const char *filename) {
    if (!filename) return NULL;
    
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return NULL;
    
    return strdup(dot + 1);
}

void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

int is_safe_path(const char *path) {
    if (!path) return 0;
    
    // Check for directory traversal attempts
    if (strstr(path, "..") != NULL) return 0;
    if (strstr(path, "//") != NULL) return 0;
    if (path[0] == '/') return 0; // Absolute paths not allowed
    
    return 1;
}

char *sanitize_filename(const char *filename) {
    if (!filename) return NULL;
    
    size_t len = strlen(filename);
    char *sanitized = malloc(len + 1);
    if (!sanitized) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = filename[i];
        if (isalnum(c) || c == '.' || c == '-' || c == '_') {
            sanitized[j++] = c;
        } else {
            sanitized[j++] = '_';
        }
    }
    
    sanitized[j] = '\0';
    return sanitized;
}

const char *get_mime_type(const char *filename) {
    if (!filename) return "application/octet-stream";
    
    const char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";
    
    ext++; // Skip the dot
    
    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) {
        return "text/html";
    } else if (strcasecmp(ext, "css") == 0) {
        return "text/css";
    } else if (strcasecmp(ext, "js") == 0) {
        return "application/javascript";
    } else if (strcasecmp(ext, "json") == 0) {
        return "application/json";
    } else if (strcasecmp(ext, "txt") == 0) {
        return "text/plain";
    } else if (strcasecmp(ext, "png") == 0) {
        return "image/png";
    } else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(ext, "gif") == 0) {
        return "image/gif";
    } else if (strcasecmp(ext, "svg") == 0) {
        return "image/svg+xml";
    } else if (strcasecmp(ext, "ico") == 0) {
        return "image/x-icon";
    }
    
    return "application/octet-stream";
}

char *get_status_message(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default:  return "Unknown";
    }
}

