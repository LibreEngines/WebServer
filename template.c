#include "template.h"
#include "logger.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

template_context_t *template_context_create(void) {
    template_context_t *context = malloc(sizeof(template_context_t));
    if (context) {
        context->variable_count = 0;
        memset(context->variables, 0, sizeof(context->variables));
    }
    return context;
}

void template_context_destroy(template_context_t *context) {
    if (context) {
        free(context);
    }
}

int template_context_set(template_context_t *context, const char *name, const char *value) {
    if (!context || !name || !value) {
        return -1;
    }
    
    // Check if variable already exists
    for (int i = 0; i < context->variable_count; i++) {
        if (strcmp(context->variables[i].name, name) == 0) {
            strncpy(context->variables[i].value, value, MAX_VAR_VALUE_SIZE - 1);
            context->variables[i].value[MAX_VAR_VALUE_SIZE - 1] = '\0';
            return 0;
        }
    }
    
    // Add new variable
    if (context->variable_count >= MAX_VARIABLES) {
        log_error("Template context is full, cannot add variable: %s", name);
        return -1;
    }
    
    strncpy(context->variables[context->variable_count].name, name, MAX_VAR_NAME_SIZE - 1);
    context->variables[context->variable_count].name[MAX_VAR_NAME_SIZE - 1] = '\0';
    
    strncpy(context->variables[context->variable_count].value, value, MAX_VAR_VALUE_SIZE - 1);
    context->variables[context->variable_count].value[MAX_VAR_VALUE_SIZE - 1] = '\0';
    
    context->variable_count++;
    return 0;
}

const char *template_context_get(template_context_t *context, const char *name) {
    if (!context || !name) {
        return NULL;
    }
    
    for (int i = 0; i < context->variable_count; i++) {
        if (strcmp(context->variables[i].name, name) == 0) {
            return context->variables[i].value;
        }
    }
    
    return NULL;
}

char *template_render_file(const char *filename, template_context_t *context) {
    if (!filename) {
        log_error("Template filename is NULL");
        return NULL;
    }
    
    char *template_content = load_file_content(filename);
    if (!template_content) {
        log_error("Failed to load template file: %s", filename);
        return NULL;
    }
    
    char *rendered = template_render_string(template_content, context);
    free(template_content);
    
    return rendered;
}

char *template_render_string(const char *template_str, template_context_t *context) {
    if (!template_str) {
        return NULL;
    }
    
    return substitute_variables(template_str, context);
}

char *substitute_variables(const char *template_str, template_context_t *context) {
    if (!template_str) {
        return NULL;
    }
    
    size_t template_len = strlen(template_str);
    size_t buffer_size = template_len * 2; // Start with double the template size
    char *result = malloc(buffer_size);
    if (!result) {
        log_error("Failed to allocate memory for template substitution");
        return NULL;
    }
    
    size_t result_pos = 0;
    const char *pos = template_str;
    
    while (*pos) {
        // Look for variable markers: {{variable_name}}
        if (pos[0] == '{' && pos[1] == '{') {
            // Find the end of the variable
            const char *var_start = pos + 2;
            const char *var_end = strstr(var_start, "}}");
            
            if (var_end) {
                // Extract variable name
                size_t var_name_len = var_end - var_start;
                char *var_name = malloc(var_name_len + 1);
                if (!var_name) {
                    log_error("Failed to allocate memory for variable name");
                    free(result);
                    return NULL;
                }
                
                strncpy(var_name, var_start, var_name_len);
                var_name[var_name_len] = '\0';
                
                // Trim whitespace from variable name
                char *trimmed_name = trim_whitespace(var_name);
                
                // Get variable value
                const char *var_value = NULL;
                if (context) {
                    var_value = template_context_get(context, trimmed_name);
                }
                
                if (!var_value) {
                    var_value = ""; // Default to empty string if variable not found
                }
                
                size_t var_value_len = strlen(var_value);
                
                // Ensure buffer is large enough
                while (result_pos + var_value_len >= buffer_size) {
                    buffer_size *= 2;
                    char *new_result = realloc(result, buffer_size);
                    if (!new_result) {
                        log_error("Failed to reallocate memory for template result");
                        free(var_name);
                        free(result);
                        return NULL;
                    }
                    result = new_result;
                }
                
                // Copy variable value to result
                memcpy(result + result_pos, var_value, var_value_len);
                result_pos += var_value_len;
                
                // Move position past the variable
                pos = var_end + 2;
                
                free(var_name);
            } else {
                // No closing }}, treat as literal text
                result[result_pos++] = *pos++;
            }
        } else {
            // Regular character, copy as-is
            if (result_pos >= buffer_size - 1) {
                buffer_size *= 2;
                char *new_result = realloc(result, buffer_size);
                if (!new_result) {
                    log_error("Failed to reallocate memory for template result");
                    free(result);
                    return NULL;
                }
                result = new_result;
            }
            
            result[result_pos++] = *pos++;
        }
    }
    
    result[result_pos] = '\0';
    return result;
}

char *load_file_content(const char *filename) {
    if (!filename) {
        return NULL;
    }
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_error("Failed to open file: %s", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0) {
        log_error("Failed to get file size: %s", filename);
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer
    char *content = malloc(file_size + 1);
    if (!content) {
        log_error("Failed to allocate memory for file content");
        fclose(file);
        return NULL;
    }
    
    // Read file content
    size_t bytes_read = fread(content, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        log_error("Failed to read complete file: %s", filename);
        free(content);
        return NULL;
    }
    
    content[file_size] = '\0';
    return content;
}

