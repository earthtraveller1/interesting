#ifndef INCLUDED_TEMPLATES_H
#define INCLUDED_TEMPLATES_H

#include "common.h"

struct template_node {
    enum {
        TEMPLATE_TEXT,
        TEMPLATE_VAR,
        TEMPLATE_IF,
        TEMPLATE_ELSE,
        TEMPLATE_FOR,
    } type;

    union {
        struct string text;
        struct string var;
    };

    // In the case of FOR expressions, the second var is the array to iterate over
    struct string second_var;

    struct template_node *children;
    size_t children_length;
    size_t children_capacity;
};

// Essentially, a root node containing all the other nodes
struct template {
    struct template_node *children;
    size_t children_length;
    size_t children_capacity;
};

struct template_error {
    enum error error;
    struct template template;
};

struct template_parameter {
    enum {
        TEMPLATE_PARAMETER_TEXT,
        TEMPLATE_PARAMETER_BOOLEAN,
    } type;

    struct string name;

    union {
        struct string text;
        bool boolean;
    };
};

struct template_parameters {
    struct template_parameter *parameters;
    size_t length;
    size_t capacity;
};

struct template parse_template(const char* source);

struct template_error parse_template_from_file(const char* filename);

void append_template_parameter(struct template_parameters *params, const struct template_parameter *param);

struct string render_template(const struct template* template, const struct template_parameter* params);

void free_template_node(const struct template_node *node);

void free_template(const struct template *template);

void free_template_parameter(const struct template_parameter *param);

void free_template_parameters(const struct template_parameters *params);

#endif
