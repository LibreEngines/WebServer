#ifndef AUTH_H
#define AUTH_H

#include <time.h>
#include <stdint.h>
#include <ctype.h>

// Forward declarations
struct http_request;
struct http_response;
typedef struct http_request http_request_t;
typedef struct http_response http_response_t;

#define MAX_USERNAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 256
#define MAX_EMAIL_LENGTH 128
#define MAX_SESSION_TOKEN_LENGTH 64
#define MAX_USERS 1000
#define MAX_SESSIONS 1000
#define SESSION_DURATION 3600  // 1 hour in seconds
#define PASSWORD_HASH_LENGTH 65  // SHA-256 hex string + null terminator

// User structure
typedef struct {
    int user_id;
    char username[MAX_USERNAME_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char password_hash[PASSWORD_HASH_LENGTH];
    char salt[33];  // 32 character salt + null terminator
    time_t created_at;
    time_t last_login;
    int is_active;
    int role;  // 0 = user, 1 = support, 2 = admin, 3 = owner
} user_t;

// Session structure
typedef struct {
    char token[MAX_SESSION_TOKEN_LENGTH];
    int user_id;
    time_t created_at;
    time_t expires_at;
    char ip_address[46];  // IPv6 compatible
    int is_valid;
} session_t;

// Authentication context
typedef struct {
    user_t users[MAX_USERS];
    session_t sessions[MAX_SESSIONS];
    int user_count;
    int session_count;
    char jwt_secret[65];  // Secret for token signing
} auth_context_t;

// Authentication functions
int auth_init(auth_context_t *auth_ctx);
void auth_cleanup(auth_context_t *auth_ctx);

// User management
int auth_register_user(auth_context_t *auth_ctx, const char *username, 
                       const char *email, const char *password);
int auth_authenticate_user(auth_context_t *auth_ctx, const char *username, 
                          const char *password);
user_t* auth_get_user_by_id(auth_context_t *auth_ctx, int user_id);
user_t* auth_get_user_by_username(auth_context_t *auth_ctx, const char *username);

// Session management
char* auth_create_session(auth_context_t *auth_ctx, int user_id, const char *ip_address);
session_t* auth_validate_session(auth_context_t *auth_ctx, const char *token);
int auth_destroy_session(auth_context_t *auth_ctx, const char *token);
void auth_cleanup_expired_sessions(auth_context_t *auth_ctx);

// Password utilities
void auth_generate_salt(char *salt, size_t length);
void auth_hash_password(const char *password, const char *salt, char *hash);
int auth_verify_password(const char *password, const char *salt, const char *hash);

// Token utilities
void auth_generate_token(char *token, size_t length);
int auth_parse_bearer_token(const char *auth_header, char *token);

// Middleware
int auth_require_login(http_request_t *request, http_response_t *response, 
                       auth_context_t *auth_ctx);
int auth_require_admin(http_request_t *request, http_response_t *response, 
                       auth_context_t *auth_ctx);

// Data persistence
int auth_save_users(auth_context_t *auth_ctx, const char *filename);
int auth_load_users(auth_context_t *auth_ctx, const char *filename);

// Utility functions
const char* auth_get_role_name(int role);
int auth_is_valid_email(const char *email);
int auth_is_valid_username(const char *username);
int auth_is_strong_password(const char *password);

#endif