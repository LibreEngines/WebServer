#include "auth.h"
#include "logger.h"
#include "utils.h"
#include "http_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Global authentication context
static auth_context_t *global_auth_ctx = NULL;

int auth_init(auth_context_t *auth_ctx) {
    if (!auth_ctx) return -1;
    
    memset(auth_ctx, 0, sizeof(auth_context_t));
    auth_ctx->user_count = 0;
    auth_ctx->session_count = 0;
    
    // Generate JWT secret
    auth_generate_token(auth_ctx->jwt_secret, sizeof(auth_ctx->jwt_secret));
    
    global_auth_ctx = auth_ctx;
    
    // Load existing users if file exists
    auth_load_users(auth_ctx, "users.dat");
    
    log_info("Authentication system initialized with %d users", auth_ctx->user_count);
    return 0;
}

void auth_cleanup(auth_context_t *auth_ctx) {
    if (!auth_ctx) return;
    
    // Save users before cleanup
    auth_save_users(auth_ctx, "users.dat");
    
    log_info("Authentication system cleaned up");
}

int auth_register_user(auth_context_t *auth_ctx, const char *username, 
                       const char *email, const char *password) {
    if (!auth_ctx || !username || !email || !password) return -1;
    
    // Validate input
    if (!auth_is_valid_username(username)) {
        log_warning("Invalid username: %s", username);
        return -2;
    }
    
    if (!auth_is_valid_email(email)) {
        log_warning("Invalid email: %s", email);
        return -3;
    }
    
    if (!auth_is_strong_password(password)) {
        log_warning("Password too weak for user: %s", username);
        return -4;
    }
    
    // Check if user already exists
    if (auth_get_user_by_username(auth_ctx, username)) {
        log_warning("User already exists: %s", username);
        return -5;
    }
    
    // Check if we have space for new user
    if (auth_ctx->user_count >= MAX_USERS) {
        log_error("Maximum users reached");
        return -6;
    }
    
    // Create new user
    user_t *user = &auth_ctx->users[auth_ctx->user_count];
    user->user_id = auth_ctx->user_count + 1;
    
    strncpy(user->username, username, MAX_USERNAME_LENGTH - 1);
    user->username[MAX_USERNAME_LENGTH - 1] = '\0';
    
    strncpy(user->email, email, MAX_EMAIL_LENGTH - 1);
    user->email[MAX_EMAIL_LENGTH - 1] = '\0';
    
    // Generate salt and hash password
    auth_generate_salt(user->salt, sizeof(user->salt));
    auth_hash_password(password, user->salt, user->password_hash);
    
    user->created_at = time(NULL);
    user->last_login = 0;
    user->is_active = 1;
    user->role = 0;  // Default to regular user
    
    auth_ctx->user_count++;
    
    // Immediately save the updated user list to disk
    if (auth_save_users(auth_ctx, "users.dat") != 0) {
        log_error("Failed to save user registration to disk");
        // Roll back the user count
        auth_ctx->user_count--;
        return -7;  // New error code for save failure
    }
    
    log_info("User registered and saved: %s (ID: %d)", username, user->user_id);
    return user->user_id;
}

int auth_authenticate_user(auth_context_t *auth_ctx, const char *username, 
                          const char *password) {
    if (!auth_ctx || !username || !password) return -1;
    
    user_t *user = auth_get_user_by_username(auth_ctx, username);
    if (!user) {
        log_warning("Authentication failed: user not found: %s", username);
        return -1;
    }
    
    if (!user->is_active) {
        log_warning("Authentication failed: user inactive: %s", username);
        return -2;
    }
    
    if (!auth_verify_password(password, user->salt, user->password_hash)) {
        log_warning("Authentication failed: wrong password for user: %s", username);
        return -3;
    }
    
    // Update last login
    user->last_login = time(NULL);
    
    log_info("User authenticated: %s", username);
    return user->user_id;
}

user_t* auth_get_user_by_id(auth_context_t *auth_ctx, int user_id) {
    if (!auth_ctx || user_id <= 0) return NULL;
    
    for (int i = 0; i < auth_ctx->user_count; i++) {
        if (auth_ctx->users[i].user_id == user_id) {
            return &auth_ctx->users[i];
        }
    }
    
    return NULL;
}

user_t* auth_get_user_by_username(auth_context_t *auth_ctx, const char *username) {
    if (!auth_ctx || !username) return NULL;
    
    for (int i = 0; i < auth_ctx->user_count; i++) {
        if (strcmp(auth_ctx->users[i].username, username) == 0) {
            return &auth_ctx->users[i];
        }
    }
    
    return NULL;
}

char* auth_create_session(auth_context_t *auth_ctx, int user_id, const char *ip_address) {
    if (!auth_ctx || user_id <= 0) return NULL;
    
    // Clean up expired sessions first
    auth_cleanup_expired_sessions(auth_ctx);
    
    // Check if we have space for new session
    if (auth_ctx->session_count >= MAX_SESSIONS) {
        log_error("Maximum sessions reached");
        return NULL;
    }
    
    // Create new session
    session_t *session = &auth_ctx->sessions[auth_ctx->session_count];
    
    auth_generate_token(session->token, sizeof(session->token));
    session->user_id = user_id;
    session->created_at = time(NULL);
    session->expires_at = session->created_at + SESSION_DURATION;
    session->is_valid = 1;
    
    if (ip_address) {
        strncpy(session->ip_address, ip_address, sizeof(session->ip_address) - 1);
        session->ip_address[sizeof(session->ip_address) - 1] = '\0';
    }
    
    auth_ctx->session_count++;
    
    log_info("Session created for user ID: %d", user_id);
    
    // Return a copy of the token
    char *token_copy = malloc(strlen(session->token) + 1);
    if (token_copy) {
        strcpy(token_copy, session->token);
    }
    
    return token_copy;
}

session_t* auth_validate_session(auth_context_t *auth_ctx, const char *token) {
    if (!auth_ctx || !token) return NULL;
    
    time_t now = time(NULL);
    
    for (int i = 0; i < auth_ctx->session_count; i++) {
        session_t *session = &auth_ctx->sessions[i];
        
        if (session->is_valid && 
            strcmp(session->token, token) == 0 && 
            now < session->expires_at) {
            return session;
        }
    }
    
    return NULL;
}

int auth_destroy_session(auth_context_t *auth_ctx, const char *token) {
    if (!auth_ctx || !token) return -1;
    
    for (int i = 0; i < auth_ctx->session_count; i++) {
        if (strcmp(auth_ctx->sessions[i].token, token) == 0) {
            auth_ctx->sessions[i].is_valid = 0;
            log_info("Session destroyed");
            return 0;
        }
    }
    
    return -1;
}

void auth_cleanup_expired_sessions(auth_context_t *auth_ctx) {
    if (!auth_ctx) return;
    
    time_t now = time(NULL);
    int expired_count = 0;
    
    for (int i = 0; i < auth_ctx->session_count; i++) {
        if (auth_ctx->sessions[i].is_valid && now >= auth_ctx->sessions[i].expires_at) {
            auth_ctx->sessions[i].is_valid = 0;
            expired_count++;
        }
    }
    
    if (expired_count > 0) {
        log_info("Cleaned up %d expired sessions", expired_count);
    }
}

void auth_generate_salt(char *salt, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    for (size_t i = 0; i < length - 1; i++) {
        salt[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    salt[length - 1] = '\0';
}

void auth_hash_password(const char *password, const char *salt, char *hash) {
    char salted_password[512];
    snprintf(salted_password, sizeof(salted_password), "%s%s", password, salt);
    
    // Simple hash function (not cryptographically secure, but functional for demo)
    unsigned long hash_value = 0;
    const char *str = salted_password;
    while (*str) {
        hash_value = hash_value * 33 + *str++;
    }
    
    // Convert to hex string (simplified to 16 characters for demo)
    snprintf(hash, PASSWORD_HASH_LENGTH, "%016lx", hash_value);
}

int auth_verify_password(const char *password, const char *salt, const char *hash) {
    char computed_hash[PASSWORD_HASH_LENGTH];
    auth_hash_password(password, salt, computed_hash);
    return strcmp(computed_hash, hash) == 0;
}

void auth_generate_token(char *token, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    for (size_t i = 0; i < length - 1; i++) {
        token[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    token[length - 1] = '\0';
}

int auth_parse_bearer_token(const char *auth_header, char *token) {
    if (!auth_header || !token) return -1;
    
    // Check if it starts with "Bearer "
    if (strncmp(auth_header, "Bearer ", 7) != 0) {
        return -1;
    }
    
    const char *token_start = auth_header + 7;
    strncpy(token, token_start, MAX_SESSION_TOKEN_LENGTH - 1);
    token[MAX_SESSION_TOKEN_LENGTH - 1] = '\0';
    
    return 0;
}

int auth_require_login(http_request_t *request, http_response_t *response, 
                       auth_context_t *auth_ctx) {
    const char *auth_header = http_request_get_header(request, "Authorization");
    
    if (!auth_header) {
        http_response_set_status(response, 401);
        http_response_set_header(response, "Content-Type", "application/json");
        http_response_set_body(response, "{\"error\":\"Authorization header required\"}");
        return -1;
    }
    
    char token[MAX_SESSION_TOKEN_LENGTH];
    if (auth_parse_bearer_token(auth_header, token) != 0) {
        http_response_set_status(response, 401);
        http_response_set_header(response, "Content-Type", "application/json");
        http_response_set_body(response, "{\"error\":\"Invalid authorization format\"}");
        return -1;
    }
    
    session_t *session = auth_validate_session(auth_ctx, token);
    if (!session) {
        http_response_set_status(response, 401);
        http_response_set_header(response, "Content-Type", "application/json");
        http_response_set_body(response, "{\"error\":\"Invalid or expired session\"}");
        return -1;
    }
    
    return session->user_id;
}

int auth_require_admin(http_request_t *request, http_response_t *response, 
                       auth_context_t *auth_ctx) {
    int user_id = auth_require_login(request, response, auth_ctx);
    if (user_id < 0) return user_id;
    
    user_t *user = auth_get_user_by_id(auth_ctx, user_id);
    if (!user || user->role != 1) {
        http_response_set_status(response, 403);
        http_response_set_header(response, "Content-Type", "application/json");
        http_response_set_body(response, "{\"error\":\"Admin access required\"}");
        return -1;
    }
    
    return user_id;
}

int auth_save_users(auth_context_t *auth_ctx, const char *filename) {
    if (!auth_ctx || !filename) return -1;
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        log_error("Failed to open users file for writing: %s", filename);
        return -1;
    }
    
    // Write user count
    fwrite(&auth_ctx->user_count, sizeof(int), 1, file);
    
    // Write users
    fwrite(auth_ctx->users, sizeof(user_t), auth_ctx->user_count, file);
    
    fclose(file);
    log_info("Saved %d users to %s", auth_ctx->user_count, filename);
    return 0;
}

int auth_load_users(auth_context_t *auth_ctx, const char *filename) {
    if (!auth_ctx || !filename) return -1;
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        log_info("Users file not found, starting with empty user database");
        return 0;  // Not an error, just no existing users
    }
    
    // Read user count
    if (fread(&auth_ctx->user_count, sizeof(int), 1, file) != 1) {
        log_error("Failed to read user count from file");
        fclose(file);
        return -1;
    }
    
    // Validate user count
    if (auth_ctx->user_count > MAX_USERS) {
        log_error("Invalid user count in file: %d", auth_ctx->user_count);
        fclose(file);
        return -1;
    }
    
    // Read users
    if (fread(auth_ctx->users, sizeof(user_t), auth_ctx->user_count, file) != 
        (size_t)auth_ctx->user_count) {
        log_error("Failed to read users from file");
        fclose(file);
        return -1;
    }
    
    fclose(file);
    log_info("Loaded %d users from %s", auth_ctx->user_count, filename);
    return 0;
}

const char* auth_get_role_name(int role) {
    switch (role) {
        case 0: return "user";
        case 1: return "support";
        case 2: return "admin";
        case 3: return "owner";
        default: return "unknown";
    }
}

int auth_is_valid_email(const char *email) {
    if (!email) return 0;
    
    size_t len = strlen(email);
    if (len < 5 || len >= MAX_EMAIL_LENGTH) return 0;
    
    // Simple email validation
    const char *at = strchr(email, '@');
    if (!at || at == email || at == email + len - 1) return 0;
    
    const char *dot = strchr(at, '.');
    if (!dot || dot == at + 1 || dot == email + len - 1) return 0;
    
    return 1;
}

int auth_is_valid_username(const char *username) {
    if (!username) return 0;
    
    size_t len = strlen(username);
    if (len < 3 || len >= MAX_USERNAME_LENGTH) return 0;
    
    // Username can only contain alphanumeric characters and underscores
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

int auth_is_strong_password(const char *password) {
    if (!password) return 0;
    
    size_t len = strlen(password);
    if (len < 8 || len >= MAX_PASSWORD_LENGTH) return 0;
    
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }
    
    // Require at least 3 of the 4 character types
    return (has_upper + has_lower + has_digit + has_special) >= 3;
}