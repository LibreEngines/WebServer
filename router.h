#ifndef ROUTER_H
#define ROUTER_H

#include <pthread.h>

// Forward declarations - these will be defined in http_server.h
struct http_request;
struct http_response;
typedef struct http_request http_request_t;
typedef struct http_response http_response_t;

typedef void (*route_handler_t)(http_request_t *request, http_response_t *response);

typedef struct route {
    char method[16];
    char pattern[1024];
    route_handler_t handler;
    struct route *next;
} route_t;

typedef struct static_route {
    char url_prefix[1024];
    char file_path[1024];
    struct static_route *next;
} static_route_t;

typedef struct {
    route_t *routes;
    static_route_t *static_routes;
    pthread_mutex_t mutex;
} router_t;

// Router functions
router_t *router_create(void);
void router_destroy(router_t *router);
int router_add_route(router_t *router, const char *method, const char *pattern, route_handler_t handler);
int router_add_static_route(router_t *router, const char *url_prefix, const char *file_path);
void router_handle_request(router_t *router, http_request_t *request, http_response_t *response);

// Route matching
int route_matches(const char *pattern, const char *path);
void handle_static_file(const char *file_path, http_response_t *response);
void handle_404(http_request_t *request, http_response_t *response);

#endif

