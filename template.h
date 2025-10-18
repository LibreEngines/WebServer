#ifndef TEMPLATE_H
#define TEMPLATE_H

#define MAX_VARIABLES 100
#define MAX_VAR_NAME_SIZE 256
#define MAX_VAR_VALUE_SIZE 4096

typedef struct {
    char name[MAX_VAR_NAME_SIZE];
    char value[MAX_VAR_VALUE_SIZE];
} template_variable_t;

typedef struct {
    template_variable_t variables[MAX_VARIABLES];
    int variable_count;
} template_context_t;

// Template context functions
template_context_t *template_context_create(void);
void template_context_destroy(template_context_t *context);
int template_context_set(template_context_t *context, const char *name, const char *value);
const char *template_context_get(template_context_t *context, const char *name);

// Template rendering functions
char *template_render_file(const char *filename, template_context_t *context);
char *template_render_string(const char *template_str, template_context_t *context);

// Helper functions
char *substitute_variables(const char *template_str, template_context_t *context);
char *load_file_content(const char *filename);

#endif

