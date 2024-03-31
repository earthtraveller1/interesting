#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "templates.h"

struct template_expression {
    enum {
        TEMPLATE_EXPRESSION_NONE,
        TEMPLATE_EXPRESSION_IF,
        TEMPLATE_EXPRESSION_ELSE,
        TEMPLATE_EXPRESSION_FOR,
        TEMPLATE_EXPRESSION_VAR,
        TEMPLATE_EXPRESSION_END,
    } type;

    struct string variable_name;
    /* Used only in the case of FOR expressions */
    struct string second_variable_name;
};

static void append_template_node(struct template_node *p_template, const struct template_node *p_node) {
    if (p_template->children_capacity == 0) {
        p_template->children_length = 0;
        p_template->children_capacity = 1;
        p_template->children = malloc(sizeof(struct template_node));
    }

    if (p_template->children_capacity == p_template->children_length) {
        p_template->children_capacity *= 2;
        p_template->children = realloc(p_template->children, p_template->children_capacity * sizeof(struct template_node));
    }

    p_template->children[p_template->children_length] = *p_node;
    p_template->children_length += 1;
}

static void append_template_node_to_template(struct template *p_template, const struct template_node *p_node) {
    if (p_template->children_capacity == 0) {
        p_template->children_length = 0;
        p_template->children_capacity = 1;
        p_template->children = malloc(sizeof(struct template_node));
    }

    if (p_template->children_capacity == p_template->children_length) {
        p_template->children_capacity *= 2;
        p_template->children = realloc(p_template->children, p_template->children_capacity * sizeof(struct template_node));
    }

    p_template->children[p_template->children_length] = *p_node;
    p_template->children_length += 1;
}

// May return NULL when the template parameter is not found.
static const struct template_parameter* get_template_parameter(const struct template_parameters* p_parameters, const char* p_name) {
    for (
        const struct template_parameter* parameter = p_parameters->parameters; 
        parameter < p_parameters->parameters + p_parameters->length; 
        parameter++
    ) {
        if (strcmp(parameter->name.data, p_name) == 0) {
            return parameter;
        }
    }

    return NULL;
}

static struct template_expression parse_expression(const char** p_source, const char* p_source_end) {
    struct template_expression expression = {0};

    char window[6] = {0};
    bool recording_variable_name = false;
    bool recorded_variable_name = false;
    bool preparing_recording_second_variable_name = false;
    bool recording_second_variable_name = false;

    while (*p_source < p_source_end) {
        memset(window, 0, 6);

        const char* character = *p_source;
        window[0] = *character;

        character += 1;
        if (character < p_source_end) {
            window[1] = *character;
        }

        character += 1;
        if (character < p_source_end) {
            window[2] = *character;
        }

        character += 1;
        if (character < p_source_end) {
            window[3] = *character;
        }

        character += 1;
        if (character < p_source_end) {
            window[4] = *character;
        }

        if (strncmp(window, "}}%", 3) == 0) {
            *p_source += 3;
            break;
        }

        if (strncmp(window, "in ", 3) == 0) {
            if (recorded_variable_name) {
                preparing_recording_second_variable_name = true;
            }
        }

        if (expression.type == TEMPLATE_EXPRESSION_NONE) {
            if (strncmp(window, "if ", 3) == 0) {
                expression.type = TEMPLATE_EXPRESSION_IF;
            }
            if (strncmp(window, "for ", 4) == 0) {
                expression.type = TEMPLATE_EXPRESSION_FOR;
            }
            if (strncmp(window, "end ", 4) == 0) {
                expression.type = TEMPLATE_EXPRESSION_END;
            }
            if (strncmp(window, "else ", 5) == 0) {
                expression.type = TEMPLATE_EXPRESSION_ELSE;
            }
            if (window[0] == '$') {
                expression.type = TEMPLATE_EXPRESSION_VAR;
                recording_variable_name = true;
            }
        } else {
            if (recording_variable_name) {
                if (**p_source == ' ' && !recorded_variable_name) {
                    recording_variable_name = false;
                    recorded_variable_name = true;
                } else {
                    string_append_char(&expression.variable_name, **p_source);
                }
            } else if (recording_second_variable_name) {
                if (**p_source == ' ') {
                    recording_second_variable_name = false;
                } else {
                    string_append_char(&expression.second_variable_name, **p_source);
                }
            } else {
                if (**p_source == '$') {
                    if (preparing_recording_second_variable_name) {
                        recording_second_variable_name = true;
                        preparing_recording_second_variable_name = false;
                    } else {
                        recording_variable_name = true;
                    }
                }
            }
        }

        *p_source += 1;
    }

    return expression;
}

struct template_node parse_node(const struct template_expression* p_first_expression, const char** p_source, const char* const p_source_end) {
    struct template_node node = {0};
    char window[4] = {0};

    if (p_first_expression->type == TEMPLATE_EXPRESSION_NONE) {
        return node;
    }

    if (p_first_expression->type == TEMPLATE_EXPRESSION_VAR) {
        node.type = TEMPLATE_VAR;
        node.var = p_first_expression->variable_name;
        return node;
    } 

    if (p_first_expression->type == TEMPLATE_EXPRESSION_ELSE) {
        node.type = TEMPLATE_ELSE;
    }

    switch (p_first_expression->type) {
        case TEMPLATE_EXPRESSION_IF:
            node.type = TEMPLATE_IF;
            break;
        case TEMPLATE_EXPRESSION_FOR:
            node.type = TEMPLATE_FOR;
            break;
        default:
            break;
    }

    node.var = p_first_expression->variable_name;
    node.second_var = p_first_expression->second_variable_name;

    struct template_node body_text_node = {
        .type = TEMPLATE_TEXT
    };

    while (*p_source < p_source_end) {
        memset(window, 0, 4);
        const char* character = *p_source;

        window[0] = *character;
        character += 1;
        if (character < p_source_end) {
            window[1] = *character;
        }

        character += 1;
        if (character < p_source_end) {
            window[2] = *character;
        }

        if (strcmp(window, "%{{") == 0) {
            if (body_text_node.text.data != NULL) {
                append_template_node(&node, &body_text_node);
                body_text_node = (struct template_node) {0};
            }

            struct template_expression expression = parse_expression(p_source, p_source_end);
            if (expression.type == TEMPLATE_EXPRESSION_END) {
                break;
            }

            struct template_node expression_node = parse_node(&expression, p_source, p_source_end);
            append_template_node(&node, &expression_node);
        } else {
            string_append_char(&body_text_node.text, **p_source);
            *p_source += 1;
        }
    }

    if (body_text_node.text.data != NULL) {
        append_template_node(&node, &body_text_node);
        body_text_node = (struct template_node) {0};
    }

    return node;
}

struct template parse_template(const char* source) {
    struct template template = {0};
    /* Remember, this points to a null character */
    const char* const source_end = source + strlen(source);

    char window[4] = {0};

    struct template_node text_node = {
        .type = TEMPLATE_TEXT
    };

    while (source < source_end) {
        const char* character = source;
        window[0] = *character;

        character += 1;
        if (character < source_end) {
            window[1] = *character;
        }

        character += 1;
        if (character < source_end) {
            window[2] = *character;
        }

        if (strcmp(window, "%{{") == 0) {
            if (text_node.text.data != NULL) {
                append_template_node_to_template(&template, &text_node);
                text_node = (struct template_node) {0};
            }

            struct template_expression expression = parse_expression(&source, source_end);
            struct template_node expression_node = parse_node(&expression, &source, source_end);
            append_template_node_to_template(&template, &expression_node);
        } else {
            string_append_char(&text_node.text, *source);
            source += 1;
        }
    }

    if (text_node.text.data != NULL) {
        append_template_node_to_template(&template, &text_node);
        text_node = (struct template_node) {0};
    }

    return template;
}

struct template_error parse_template_from_file(const char* p_filename) {
    struct template_error result = {0};

    struct string_error file_contents = read_string_from_file(p_filename);
    if (file_contents.error != INTERESTING_ERROR_SUCCESS) {
        result.error = file_contents.error;

        // We don't have to free the string here, as it should be null
        return result;
    }

    result.template = parse_template(file_contents.string.data);
    free_string(&file_contents.string);
    return result;
}

void append_template_parameter(struct template_parameters *p_parameters, const struct template_parameter *p_parameter) {
    if (p_parameters->capacity == 0) {
        p_parameters->length = 0;
        p_parameters->capacity = 1;
        p_parameters->parameters = malloc(sizeof(struct template_parameter));
    }

    if (p_parameters->capacity == p_parameters->length) {
        p_parameters->capacity *= 2;
        p_parameters->parameters = realloc(p_parameters->parameters, p_parameters->capacity * sizeof(struct template_parameter));
    }

    p_parameters->parameters[p_parameters->length] = *p_parameter;
    p_parameters->length += 1;
}

enum error render_node(
    const struct template_node* p_node, 
    const struct template_node* p_previous_node,
    const struct template_parameters* params, 
    struct string* p_target
) {
    #define render_children(params) \
        do { \
            for ( \
                const struct template_node* node = p_node->children, *previous_node = NULL; \
                node < p_node->children + p_node->children_length; \
                node++, previous_node = node - 1 \
            ) { \
                const enum error result = render_node(node, previous_node, params, p_target); \
 \
                if (result != INTERESTING_ERROR_SUCCESS) { \
                    return result; \
                } \
            } \
        } while (0)

    switch (p_node->type) {
        case TEMPLATE_TEXT:
            string_concat(p_target, &p_node->text);
            break;
        case TEMPLATE_VAR:
            {
                const struct template_parameter* variable = get_template_parameter(params, p_node->var.data);
                if (variable == NULL) {
                    return ERROR_REQUIRED_PARAMETER_NOT_PROVIDED;
                }

                if (variable->type != TEMPLATE_PARAMETER_TEXT) {
                    return ERROR_PARAMETER_WRONG_TYPE;
                }

                string_concat(p_target, &variable->text);
                break;
            }
        case TEMPLATE_IF:
            {
                const struct template_parameter* variable = get_template_parameter(params, p_node->var.data);
                if (variable == NULL) {
                    return ERROR_REQUIRED_PARAMETER_NOT_PROVIDED;
                }

                if (variable->type != TEMPLATE_PARAMETER_BOOLEAN) {
                    return ERROR_PARAMETER_WRONG_TYPE;
                }

                if (variable->boolean) {
                    render_children(params);
                }

                break;
            }
        case TEMPLATE_ELSE:
            {
                if (p_previous_node->type != TEMPLATE_IF) {
                    return ERROR_ELSE_MUST_FOLLOW_IF;
                }

                const struct template_parameter* if_variable = get_template_parameter(params, p_previous_node->var.data);
                if (if_variable == NULL) {
                    return ERROR_REQUIRED_PARAMETER_NOT_PROVIDED;
                }

                if (if_variable->type != TEMPLATE_PARAMETER_BOOLEAN) {
                    return ERROR_PARAMETER_WRONG_TYPE;
                }

                if (!if_variable->boolean) {
                    render_children(params);
                }

                break;
            }
        case TEMPLATE_FOR:
            {
                const struct template_parameter* list_variable = get_template_parameter(params, p_node->second_var.data);
                if (list_variable == NULL) {
                    return ERROR_REQUIRED_PARAMETER_NOT_PROVIDED;
                }

                if (list_variable->type != TEMPLATE_PARAMETER_LIST) {
                    return ERROR_PARAMETER_WRONG_TYPE;
                }

                struct template_parameters new_parameters = {0};

                for (
                    const struct template_parameter* param = params->parameters;
                    param < params->parameters + params->length;
                    param++
                ) {
                    if (strcmp(param->name.data, p_node->var.data) != 0) {
                        append_template_parameter(&new_parameters, param);
                    }
                }

                for (
                    const struct string* list_item = list_variable->list.strings;
                    list_item < list_variable->list.strings + list_variable->list.length;
                    list_item++
                ) {
                    append_template_parameter(&new_parameters, &(struct template_parameter) {
                        .name = p_node->var,
                        .type = TEMPLATE_PARAMETER_TEXT,
                        .text = *list_item,
                    });

                    render_children(&new_parameters);

                    new_parameters.length -= 1;
                }

                /* We do this to avoid freeing the individual parameters, as we don't own them here! */
                new_parameters.length = 0;
                free_template_parameters(&new_parameters);

                break;
            }
    }

    #undef render_children

    return INTERESTING_ERROR_SUCCESS;
}

struct string render_template(const struct template* template, const struct template_parameters* params) {
    struct string rendered_result = {0};

    for (
        const struct template_node* node = template->children, *previous_node = NULL;
        node < template->children + template->children_length;
        node++, previous_node = node - 1
    ) {
        render_node(node, previous_node, params, &rendered_result);
    }

    return rendered_result;
}

void free_template_node(const struct template_node* p_node) {
    // We don't needd to free var separately, 
    // as they are in a union, so this should free both of them.
    free_string(&p_node->text);

    for (const struct template_node* node = p_node->children; node < p_node->children + p_node->children_length; node++) {
        free_template_node(node);
    }

    free(p_node->children);
}

void free_template(const struct template* p_template) {
    for (const struct template_node* node = p_template->children; node < p_template->children + p_template->children_length; node++) {
        free_template_node(node);
    }

    free(p_template->children);
}

void free_template_parameter(const struct template_parameter* p_parameter) {
    if (p_parameter->type == TEMPLATE_PARAMETER_TEXT) {
        free_string(&p_parameter->text);
    }
}

void free_template_parameters(const struct template_parameters* p_parameters) {
    for (const struct template_parameter* parameter = p_parameters->parameters; parameter < p_parameters->parameters + p_parameters->length; parameter++) {
        free_template_parameter(parameter);
    }

    free(p_parameters->parameters);
}
