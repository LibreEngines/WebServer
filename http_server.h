#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "router.h"

#define MAX_HEADERS 50
#define MAX_HEADER_SIZE 1024
#define MAX_BODY_SIZE 65536
#define MAX_REQUEST_SIZE 65536
#define MAX_THREADS 100

typedef struct {
    char name[256];
    char value[1024];
} http_header_t;

typedef struct http_request {
    char method[16];
    char path[1024];
    char version[16];
    http_header_t headers[MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_length;
} http_request_t;

typedef struct http_response {
    int status_code;
    http_header_t headers[MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_length;
} http_response_t;

typedef struct {
    char host[256];
    int port;
    int socket_fd;
    struct sockaddr_in address;
    router_t *router;
    pthread_mutex_t mutex;
    int running;
} http_server_t;

typedef struct {
    http_server_t *server;
    int client_socket;
} client_handler_args_t;

// Server functions
http_server_t *http_server_create(const char *host, int port);
int http_server_start(http_server_t *server);
void http_server_stop(http_server_t *server);
void http_server_destroy(http_server_t *server);

// Request/Response functions
http_request_t *http_request_create(void);
void http_request_destroy(http_request_t *request);
const char *http_request_get_header(http_request_t *request, const char *name);
const char *http_request_get_body(http_request_t *request);

http_response_t *http_response_create(void);
void http_response_destroy(http_response_t *response);
void http_response_set_status(http_response_t *response, int status);
void http_response_set_header(http_response_t *response, const char *name, const char *value);
void http_response_set_body(http_response_t *response, const char *body);

// HTTP parsing
int http_parse_request(const char *raw_request, http_request_t *request);
char *http_serialize_response(http_response_t *response);

// Client handling
void *handle_client(void *args);

#endif

