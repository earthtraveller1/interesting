#include <stdlib.h>

#include "templates.h"

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
