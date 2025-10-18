#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include "http_server.h"
#include "router.h"
#include "logger.h"
#include "template.h"
#include "auth.h"
#include "addresses.h"

// Global server instance for signal handling
http_server_t *global_server = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////
// Maintenance Bool Variable
bool maintenanceMode = false;
////////////////////////////////////////////////////////////////////////////////////////////////
// Global authentication context
auth_context_t auth_context;
////////////////////////////////////////////////////////////////////////////////////////////////s


// Signal handler for graceful shutdown
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        log_info("Received shutdown signal, stopping server...");
        if (global_server) {
            http_server_stop(global_server);
        }
        exit(0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
void handle_maintenance(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "maintenance_title", "LibreGrad: Maintenance");
    
    char *rendered = template_render_file("templates/utils/maintenance.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}
////////////////////////////////////////////////////////////////////////////////////////////////
// Route handlers
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_home(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "title", "LibreGrad Engines");    
    char *rendered = template_render_file("templates/landing2.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_blog(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    
    char *rendered = template_render_file("templates/blog.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// Blogs
////////////////////////////////////////////////////////////////////////////////////////////////
// Engine Design Process Blog
void handle_engine_design_process_blog(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "title", "The Engine Design Process: From Concept to Creation");
    
    char *rendered = template_render_file("templates/blogs/engine-design-process.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}

/// End Blogs
////////////////////////////////////////////////////////////////////////////////////////////////

void handle_engines_page(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    // Our Engines Products
    template_context_set(ctx, "title", "Our Engines");
    template_context_set(ctx, "message", "Welcome to our engines page, here you can find all our available engines products.");
    template_context_set(ctx, "engine_50_r1_title", "Engine R1-50CC");
    template_context_set(ctx, "engine_50_r1_desc", "The Engine R1-50CC is a powerful and efficient engine designed for high performance and reliability. It features advanced technology to ensure optimal fuel consumption and reduced emissions.");
    template_context_set(ctx, "engine_50_r1_price", "$499.99");



    template_context_set(ctx, "engine_50_r1_specs_title", "Specifications");
    // template_context_set(ctx, "engine_50_r1_specs", 
    // "Displacement: 49.5cc\n\
    // Type: Single-cylinder, 2-stroke\n\
    // Cooling System: Air-cooled\n\
    // Bore x Stroke: 40.0 mm x 39.2 mm\n\
    // Compression Ratio: 10.5:1\n\
    // Fuel System: Carbureted (19mm performance carb)\n\
    // Ignition: CDI (Capacitor Discharge Ignition)\n\
    // Starting System: Kick-start & optional electric start\n\
    // Lubrication: Pre-mix (40:1 ratio)\n\
    // Transmission: Manual, 4-speed\n\
    // Clutch: Wet multi-plate\n\
    // Crankshaft: Forged steel, balanced\n\
    // Piston: Lightweight, cast aluminum alloy\n\
    // Exhaust: Tuned expansion chamber with silencer\n\
    // Max Power: 7.5 HP @ 9,000 RPM\n\
    // Max Torque: 6.2 Nm @ 7,000 RPM\n\
    // Top Speed: ~25 km/h (restricted), up to 85 km/h (unrestricted)\n\
    // Weight: 18.5 kg (engine only)\n\
    // Emissions: Euro 4 compliant\n\
    // Fuel Consumption: ~1.8 L/100km\n\
    // Recommended Fuel: Unleaded 91+ RON\n\
    // Recommended Oil: Fully synthetic 2T oil\n\
    // Maintenance Interval: 20 hours (initial), 50 hours (routine)\n\
    // Use Case: Mopeds, small motorcycles, scooters, pit bikes");

    template_context_set(ctx, "engine_50_r1_specs", 
    "Displacement: 49.5cc\n\
    Type: Single-cylinder, 2-stroke\n\
    Cooling System: Air-cooled\n\
    Bore x Stroke: 40.0 mm x 39.2 mm\n\
    Compression Ratio: 10.5:1\n\
    Fuel System: Carbureted (19mm performance carb)\n\
    Ignition: CDI (Capacitor Discharge Ignition)\n\
    Starting System: Kick-start & optional electric start\n\
    Lubrication: Pre-mix (40:1 ratio)\n\
    Transmission: Manual, 4-speed\n\
    Clutch: Wet multi-plate\n\
    Crankshaft: Forged steel, balanced\n\
    Piston: Lightweight, cast aluminum alloy\n\
    Exhaust: Tuned expansion chamber with silencer\n\
    Max Power: 4.0 HP @ 6,500 RPM (restricted)\n\
    Max Torque: 4.5 Nm @ 5,500 RPM\n\
    Top Speed: 25 km/h (electronically and mechanically restricted)\n\
    Restriction Method: Rev limiter + gear ratio tuning + intake restrictor\n\
    Weight: 18.5 kg (engine only)\n\
    Emissions: Euro 4 compliant\n\
    Fuel Consumption: ~1.5 L/100km\n\
    Recommended Fuel: Unleaded 91+ RON\n\
    Recommended Oil: Fully synthetic 2T oil\n\
    Maintenance Interval: 20 hours (initial), 50 hours (routine)\n\
    Use Case: Legal urban mopeds, low-speed recreational vehicles, youth training bikes");


    template_context_set(ctx, "engine_50_r1_usecases", "Ideal for mopeds, pit bikes, small scooters, and light off-road vehicles.");
    template_context_set(ctx, "engine_50_r1_warranty", "Includes a 12-month manufacturer warranty with full parts support.");
    template_context_set(ctx, "engine_50_r1_availability", "In Stock — Ready to ship");
    template_context_set(ctx, "engine_50_r1_shipping", "Free shipping. International shipping available.");

    template_context_set(ctx, "engine_50_r1_image_main", "/images/engine_50_r1_main.jpg");
    template_context_set(ctx, "engine_50_r1_image_thumb_1", "/images/engine_50_r1_side.jpg");
    template_context_set(ctx, "engine_50_r1_image_thumb_2", "/images/engine_50_r1_top.jpg");

    template_context_set(ctx, "engine_50_r1_meta_title", "Engine R1-50CC | 49.5cc 4-Stroke Engine | In-House Build");
    template_context_set(ctx, "engine_50_r1_meta_desc", "Buy the Engine R1-50CC – a reliable, restricted 4-stroke 50cc engine designed for mopeds, scooters, and pit bikes. Euro 4 compliant. Free US shipping.");


    char *rendered = template_render_file("templates/engines.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_server(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "title", "Our Server");
    template_context_set(ctx, "message", "Welcome to our server info, this is the server running the dashboard.");
    template_context_set(ctx, "version", "1.0.0");
    template_context_set(ctx, "status", "ALPHA");
    
    char *rendered = template_render_file("templates/server.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}


////////////////////////////////////////////////////////////////////////////////////////////////
void auth_handler(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    
    char *rendered = template_render_file("templates/auth/auth.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Unused - For Now
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_api_status(http_request_t *request, http_response_t *response) {
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "status", "running");
    template_context_set(ctx, "uptime", "available");
    
    char *json_response = "{\n  \"status\": \"running\",\n  \"server\": \"Advanced C Web Server\",\n  \"version\": \"1.0.0\"\n}";
    
    http_response_set_body(response, json_response);
    http_response_set_header(response, "Content-Type", "application/json");
    http_response_set_status(response, 200);
    
    template_context_destroy(ctx);
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// Unused
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_post_data(http_request_t *request, http_response_t *response) {
    const char *body = http_request_get_body(request);
    
    template_context_t *ctx = template_context_create();
    template_context_set(ctx, "title", "POST Data Received");
    template_context_set(ctx, "data", body ? body : "No data received");
    
    char *rendered = template_render_file("templates/index.html", ctx);
    if (rendered) {
        http_response_set_body(response, rendered);
        http_response_set_header(response, "Content-Type", "text/html");
        http_response_set_status(response, 200);
        free(rendered);
    } else {
        http_response_set_status(response, 500);
        http_response_set_body(response, "Internal Server Error");
    }
    
    template_context_destroy(ctx);
}
////////////////////////////////////////////////////////////////////////////////////////////////
// Authentication route handlers - Unused
////////////////////////////////////////////////////////////////////////////////////////////////
void trim_quotes(char *str) {
    if (!str) return;
    
    size_t len = strlen(str);
    if (len >= 2) {
        if ((str[0] == '"' && str[len-1] == '"') || 
            (str[0] == '\'' && str[len-1] == '\'')) {
            memmove(str, str+1, len-2);
            str[len-2] = '\0';
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_register(http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        http_response_set_status(response, 405);
        http_response_set_body(response, "{\"error\":\"Method not allowed\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }

    // Parse JSON body
    const char *body = http_request_get_body(request);
    if (!body || strlen(body) == 0) {
        http_response_set_status(response, 400);
        http_response_set_body(response, "{\"error\":\"Request body required\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }

    log_info("Registration request body: %s", body);

    // More flexible JSON parsing
    char username[MAX_USERNAME_LENGTH] = {0};
    char email[MAX_EMAIL_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};

    // Try multiple JSON parsing patterns
    int parsed = 0;
    
    // Pattern 1: Strict JSON with quotes
    if (sscanf(body, "{\"username\":\"%[^\"]\",\"email\":\"%[^\"]\",\"password\":\"%[^\"]\"}", 
               username, email, password) == 3) {
        parsed = 1;
    }
    // Pattern 2: More flexible with whitespace
    else if (sscanf(body, "{%*[ \t\n\r]\"username\"%*[ \t\n\r]:%*[ \t\n\r]\"%[^\"]\"%*[ \t\n\r],%*[ \t\n\r]\"email\"%*[ \t\n\r]:%*[ \t\n\r]\"%[^\"]\"%*[ \t\n\r],%*[ \t\n\r]\"password\"%*[ \t\n\r]:%*[ \t\n\r]\"%[^\"]\"", 
                    username, email, password) == 3) {
        parsed = 1;
    }
    // Pattern 3: Single quotes
    else if (sscanf(body, "{'username':'%[^']','email':'%[^']','password':'%[^']'}", 
                    username, email, password) == 3) {
        parsed = 1;
    }
    // Pattern 4: No quotes around values
    else if (sscanf(body, "{\"username\":%[^,],\"email\":%[^,],\"password\":%[^}]}", 
                    username, email, password) == 3) {
        // Remove potential quotes from values
        trim_quotes(username);
        trim_quotes(email);
        trim_quotes(password);
        parsed = 1;
    }

    if (!parsed) {
        http_response_set_status(response, 400);
        http_response_set_body(response, "{\"error\":\"Invalid JSON format\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }

    log_info("Parsed registration: username='%s' email='%s' password='%s'", 
             username, email, password);

    // Proceed with registration
    int result = auth_register_user(&auth_context, username, email, password);
    
    char response_body[512];
    if (result > 0) {
        snprintf(response_body, sizeof(response_body), 
                "{\"success\":true,\"message\":\"User registered successfully\",\"user_id\":%d}", 
                result);
        http_response_set_status(response, 201);
    } else {
        const char *error_msg;
        switch (result) {
            case -2: error_msg = "Invalid username"; break;
            case -3: error_msg = "Invalid email"; break;
            case -4: error_msg = "Password too weak"; break;
            case -5: error_msg = "User already exists"; break;
            case -6: error_msg = "Maximum users reached"; break;
            default: error_msg = "Registration failed"; break;
        }
        snprintf(response_body, sizeof(response_body), 
                "{\"success\":false,\"error\":\"%s\"}", error_msg);
        http_response_set_status(response, 400);
    }
    
    http_response_set_body(response, response_body);
    http_response_set_header(response, "Content-Type", "application/json");
    
    // Add CORS headers if needed
    http_response_set_header(response, "Access-Control-Allow-Origin", "*");
    http_response_set_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    http_response_set_header(response, "Access-Control-Allow-Headers", "Content-Type");
}
////////////////////////////////////////////////////////////////////////////////////////////////

void handle_login(http_request_t *request, http_response_t *response) {
    if (strcmp(request->method, "POST") != 0) {
        http_response_set_status(response, 405);
        http_response_set_body(response, "{\"error\":\"Method not allowed\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    const char *body = http_request_get_body(request);
    if (!body) {
        http_response_set_status(response, 400);
        http_response_set_body(response, "{\"error\":\"Request body required\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    char username[MAX_USERNAME_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};
    
    char *username_start = strstr(body, "\"username\":");
    char *password_start = strstr(body, "\"password\":");
    
    if (!username_start || !password_start) {
        http_response_set_status(response, 400);
        http_response_set_body(response, "{\"error\":\"Username and password required\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    if (sscanf(username_start, "\"username\":\"%63[^\"]\"", username) != 1 ||
        sscanf(password_start, "\"password\":\"%255[^\"]\"", password) != 1) {
        http_response_set_status(response, 400);
        http_response_set_body(response, "{\"error\":\"Invalid JSON format\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    int user_id = auth_authenticate_user(&auth_context, username, password);
    
    char response_body[512];
    if (user_id > 0) {
        char *session_token = auth_create_session(&auth_context, user_id, "127.0.0.1");
        if (session_token) {
            user_t *user = auth_get_user_by_id(&auth_context, user_id);
            snprintf(response_body, sizeof(response_body), 
                    "{\"success\":true,\"token\":\"%s\",\"user\":{\"id\":%d,\"username\":\"%s\",\"role\":\"%s\"}}", 
                    session_token, user_id, user->username, auth_get_role_name(user->role));
            free(session_token);
            http_response_set_status(response, 200);
        } else {
            snprintf(response_body, sizeof(response_body), 
                    "{\"success\":false,\"error\":\"Session creation failed\"}");
            http_response_set_status(response, 500);
        }
    } else {
        snprintf(response_body, sizeof(response_body), 
                "{\"success\":false,\"error\":\"Invalid credentials\"}");
        http_response_set_status(response, 401);
    }
    
    http_response_set_body(response, response_body);
    http_response_set_header(response, "Content-Type", "application/json");
}
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_logout(http_request_t *request, http_response_t *response) {
    const char *auth_header = http_request_get_header(request, "Authorization");
    
    if (!auth_header) {
        http_response_set_status(response, 401);
        http_response_set_body(response, "{\"error\":\"Authorization header required\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    char token[MAX_SESSION_TOKEN_LENGTH];
    if (auth_parse_bearer_token(auth_header, token) != 0) {
        http_response_set_status(response, 401);
        http_response_set_body(response, "{\"error\":\"Invalid authorization format\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    if (auth_destroy_session(&auth_context, token) == 0) {
        http_response_set_status(response, 200);
        http_response_set_body(response, "{\"success\":true,\"message\":\"Logged out successfully\"}");
    } else {
        http_response_set_status(response, 400);
        http_response_set_body(response, "{\"success\":false,\"error\":\"Session not found\"}");
    }
    
    http_response_set_header(response, "Content-Type", "application/json");
}
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_profile(http_request_t *request, http_response_t *response) {
    int user_id = auth_require_login(request, response, &auth_context);
    if (user_id < 0) return;  // Response already set by auth_require_login
    
    user_t *user = auth_get_user_by_id(&auth_context, user_id);
    if (!user) {
        http_response_set_status(response, 404);
        http_response_set_body(response, "{\"error\":\"User not found\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    char response_body[1024];
    snprintf(response_body, sizeof(response_body), 
            "{\"success\":true,\"user\":{\"id\":%d,\"username\":\"%s\",\"email\":\"%s\",\"role\":\"%s\",\"created_at\":%ld,\"last_login\":%ld}}", 
            user->user_id, user->username, user->email, auth_get_role_name(user->role), 
            user->created_at, user->last_login);
    
    http_response_set_status(response, 200);
    http_response_set_body(response, response_body);
    http_response_set_header(response, "Content-Type", "application/json");
}
////////////////////////////////////////////////////////////////////////////////////////////////
void handle_users(http_request_t *request, http_response_t *response) {
    int user_id = auth_require_admin(request, response, &auth_context);
    if (user_id < 0) return;  // Response already set by auth_require_admin
    
    // Build JSON array of users (admin endpoint)
    char *response_body = malloc(8192);
    if (!response_body) {
        http_response_set_status(response, 500);
        http_response_set_body(response, "{\"error\":\"Memory allocation failed\"}");
        http_response_set_header(response, "Content-Type", "application/json");
        return;
    }
    
    strcpy(response_body, "{\"success\":true,\"users\":[");
    
    for (int i = 0; i < auth_context.user_count; i++) {
        user_t *user = &auth_context.users[i];
        char user_json[512];
        snprintf(user_json, sizeof(user_json), 
                "%s{\"id\":%d,\"username\":\"%s\",\"email\":\"%s\",\"role\":\"%s\",\"created_at\":%ld,\"last_login\":%ld,\"is_active\":%s}",
                i > 0 ? "," : "", user->user_id, user->username, user->email, 
                auth_get_role_name(user->role), user->created_at, user->last_login,
                user->is_active ? "true" : "false");
        strcat(response_body, user_json);
    }
    
    strcat(response_body, "]}");
    
    http_response_set_status(response, 200);
    http_response_set_body(response, response_body);
    http_response_set_header(response, "Content-Type", "application/json");
    
    free(response_body);
}
////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    // Initialize logger
    if (logger_init("server.log") != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        return 1;
    }
    
    log_info("Starting Advanced C Web Server...");
    
    // Initialize authentication system
    if (auth_init(&auth_context) != 0) {
        log_error("Failed to initialize authentication system");
        return 1;
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create and configure server
    http_server_t *server = http_server_create("0.0.0.0", 5000);
    if (!server) {
        log_error("Failed to create server");
        return 1;
    }
    
    global_server = server;
    
    // Setup routes
    // router_add_route(server->router, "GET", "/a", auth_handler);
    // router_add_route(server->router, "GET", "/api/status", handle_api_status);
    // router_add_route(server->router, "POST", "/submit", handle_post_data);
    router_add_route(server->router, "GET", "/server", handle_server);
    router_add_route(server->router, "GET", "/blog", handle_blog);

    // Blogs
    router_add_route(server->router, "GET", "/blogs/engine-design-process", handle_engine_design_process_blog);

    
    // Authentication routes
    // router_add_route(server->router, "POST", "/api/register", handle_register);
    // router_add_route(server->router, "POST", "/api/login", handle_login);
    // router_add_route(server->router, "POST", "/api/logout", handle_logout);
    // router_add_route(server->router, "GET", "/api/profile", handle_profile);
    // router_add_route(server->router, "GET", "/api/users", handle_users);
    
    // Maintenance Check.
    if (maintenanceMode) {
        router_add_route(server->router, "GET", "/", handle_maintenance);
    }
    else {
        router_add_route(server->router, "GET", "/", handle_home);
    }

    // Enable static file serving
    router_add_static_route(server->router, "/static", "static");
    
    log_info("Server configured with routes:");
    log_info("  GET  / - Home page");
    log_info("  GET  /api/status - Server status API");
    log_info("  POST /submit - Handle form submissions");
    log_info("  POST /api/register - User registration");
    log_info("  POST /api/login - User authentication");
    log_info("  POST /api/logout - User logout");
    log_info("  GET  /api/profile - User profile (requires auth)");
    log_info("  GET  /api/users - List all users (admin only)");
    log_info("  Static files served from /static");
    
    // Start server
    if (http_server_start(server) != 0) {
        log_error("Failed to start server");
        http_server_destroy(server);
        return 1;
    }
    
    log_info("Server started successfully on http://0.0.0.0:5000");
    
    // Keep main thread alive
    while (1) {
        sleep(1);
    }
    
    // Cleanup
    auth_cleanup(&auth_context);
    http_server_destroy(server);
    logger_cleanup();
    
    return 0;
}
