#include "router.h"
#include "http_server.h"
#include "template.h"
#include "logger.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

router_t *router_create(void) {
    router_t *router = malloc(sizeof(router_t));
    if (!router) {
        log_error("Failed to allocate memory for router");
        return NULL;
    }
    
    router->routes = NULL;
    router->static_routes = NULL;
    
    if (pthread_mutex_init(&router->mutex, NULL) != 0) {
        log_error("Failed to initialize router mutex");
        free(router);
        return NULL;
    }
    
    return router;
}

void router_destroy(router_t *router) {
    if (!router) return;
    
    pthread_mutex_lock(&router->mutex);
    
    // Free routes
    route_t *current_route = router->routes;
    while (current_route) {
        route_t *next = current_route->next;
        free(current_route);
        current_route = next;
    }
    
    // Free static routes
    static_route_t *current_static = router->static_routes;
    while (current_static) {
        static_route_t *next = current_static->next;
        free(current_static);
        current_static = next;
    }
    
    pthread_mutex_unlock(&router->mutex);
    pthread_mutex_destroy(&router->mutex);
    free(router);
}

int router_add_route(router_t *router, const char *method, const char *pattern, route_handler_t handler) {
    if (!router || !method || !pattern || !handler) {
        return -1;
    }
    
    route_t *new_route = malloc(sizeof(route_t));
    if (!new_route) {
        log_error("Failed to allocate memory for route");
        return -1;
    }
    
    strncpy(new_route->method, method, sizeof(new_route->method) - 1);
    new_route->method[sizeof(new_route->method) - 1] = '\0';
    
    strncpy(new_route->pattern, pattern, sizeof(new_route->pattern) - 1);
    new_route->pattern[sizeof(new_route->pattern) - 1] = '\0';
    
    new_route->handler = handler;
    new_route->next = NULL;
    
    pthread_mutex_lock(&router->mutex);
    
    if (!router->routes) {
        router->routes = new_route;
    } else {
        route_t *current = router->routes;
        while (current->next) {
            current = current->next;
        }
        current->next = new_route;
    }
    
    pthread_mutex_unlock(&router->mutex);
    
    log_info("Added route: %s %s", method, pattern);
    return 0;
}

int router_add_static_route(router_t *router, const char *url_prefix, const char *file_path) {
    if (!router || !url_prefix || !file_path) {
        return -1;
    }
    
    static_route_t *new_static = malloc(sizeof(static_route_t));
    if (!new_static) {
        log_error("Failed to allocate memory for static route");
        return -1;
    }
    
    strncpy(new_static->url_prefix, url_prefix, sizeof(new_static->url_prefix) - 1);
    new_static->url_prefix[sizeof(new_static->url_prefix) - 1] = '\0';
    
    strncpy(new_static->file_path, file_path, sizeof(new_static->file_path) - 1);
    new_static->file_path[sizeof(new_static->file_path) - 1] = '\0';
    
    new_static->next = NULL;
    
    pthread_mutex_lock(&router->mutex);
    
    if (!router->static_routes) {
        router->static_routes = new_static;
    } else {
        static_route_t *current = router->static_routes;
        while (current->next) {
            current = current->next;
        }
        current->next = new_static;
    }
    
    pthread_mutex_unlock(&router->mutex);
    
    log_info("Added static route: %s -> %s", url_prefix, file_path);
    return 0;
}

void router_handle_request(router_t *router, http_request_t *request, http_response_t *response) {
    if (!router || !request || !response) {
        return;
    }
    
    pthread_mutex_lock(&router->mutex);
    
    // Check dynamic routes first
    route_t *current_route = router->routes;
    while (current_route) {
        if (strcmp(current_route->method, request->method) == 0 &&
            route_matches(current_route->pattern, request->path)) {
            pthread_mutex_unlock(&router->mutex);
            current_route->handler(request, response);
            return;
        }
        current_route = current_route->next;
    }
    
    // Check static routes
    static_route_t *current_static = router->static_routes;
    while (current_static) {
        if (strncmp(request->path, current_static->url_prefix, strlen(current_static->url_prefix)) == 0) {
            // Extract relative path
            const char *relative_path = request->path + strlen(current_static->url_prefix);
            if (relative_path[0] == '/') relative_path++; // Skip leading slash
            
            // Construct full file path
            char full_path[2048];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_static->file_path, relative_path);
            
            pthread_mutex_unlock(&router->mutex);
            handle_static_file(full_path, response);
            return;
        }
        current_static = current_static->next;
    }
    
    pthread_mutex_unlock(&router->mutex);
    
    // No route found, return 404
    handle_404(request, response);
}

int route_matches(const char *pattern, const char *path) {
    if (!pattern || !path) return 0;
    
    // Simple exact match for now
    // TODO: Implement wildcard and parameter matching
    return strcmp(pattern, path) == 0;
}

void handle_static_file(const char *file_path, http_response_t *response) {
    if (!file_path || !response) return;
    
    // Security check: prevent directory traversal
    if (strstr(file_path, "..") != NULL) {
        log_warning("Directory traversal attempt blocked: %s", file_path);
        http_response_set_status(response, 400);
        http_response_set_body(response, "Bad Request");
        return;
    }
    
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        log_warning("Static file not found: %s", file_path);
        http_response_set_status(response, 404);
        http_response_set_body(response, "File Not Found");
        return;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0 || file_size > MAX_BODY_SIZE) {
        log_error("File size error or too large: %s", file_path);
        fclose(file);
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
        return;
    }
    
    // Read file content
    char *content = malloc(file_size + 1);
    if (!content) {
        log_error("Failed to allocate memory for file content");
        fclose(file);
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
        return;
    }
    
    size_t bytes_read = fread(content, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        log_error("Failed to read complete file: %s", file_path);
        free(content);
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
        return;
    }
    
    content[file_size] = '\0';
    
    // Set content type based on file extension
    const char *ext = strrchr(file_path, '.');
    if (ext) {
        if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            http_response_set_header(response, "Content-Type", "text/html");
        } else if (strcmp(ext, ".css") == 0) {
            http_response_set_header(response, "Content-Type", "text/css");
        } else if (strcmp(ext, ".js") == 0) {
            http_response_set_header(response, "Content-Type", "application/javascript");
        } else if (strcmp(ext, ".json") == 0) {
            http_response_set_header(response, "Content-Type", "application/json");
        } else if (strcmp(ext, ".txt") == 0) {
            http_response_set_header(response, "Content-Type", "text/plain");
        } else {
            http_response_set_header(response, "Content-Type", "application/octet-stream");
        }
    }
    
    http_response_set_status(response, 200);
    http_response_set_body(response, content);
    
    free(content);
    log_info("Served static file: %s", file_path);
}

void handle_404(http_request_t *request, http_response_t *response) {
    log_warning("404 Not Found: %s %s", request->method, request->path);
    
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "title", "Page Not Found");
    template_context_set(ctx, "message", "The requested page could not be found.");
    template_context_set(ctx, "error_code", "404");
    
    char *rendered = template_render_file("templates/error.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        free(rendered);
    } else {
        http_response_set_body(response, "404 Not Found");
        http_response_set_header(response, "Content-Type", "text/plain");
    }
    
    http_response_set_status(response, 404);
    template_context_destroy(ctx);
}

