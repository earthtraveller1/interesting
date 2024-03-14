#include <string.h>
#include <stdlib.h>

#include "router.h"

static void append_route_parts(struct route* p_route, const struct route_part* p_part) {
    if (p_route->parts_capacity == 0) {
        p_route->parts_length = 0;
        p_route->parts_capacity = 1;
        p_route->parts = malloc(sizeof(struct route_part));
    }

    if (p_route->parts_capacity == p_route->parts_length) {
        p_route->parts_capacity *= 2;
        p_route->parts = realloc(p_route->parts, p_route->parts_capacity * sizeof(struct route_part));
    }

    p_route->parts[p_route->parts_length] = *p_part;
    p_route->parts_length += 1;
}

static void append_parameters(struct parameters* p_parameters, const struct parameter* p_parameter) {
    if (p_parameters->capacity == 0) {
        p_parameters->length = 0;
        p_parameters->capacity = 1;
        p_parameters->list = malloc(sizeof(struct parameter));
    }

    if (p_parameters->capacity == p_parameters->length) {
        p_parameters->capacity *= 2;
        p_parameters->list = realloc(p_parameters->list, p_parameters->capacity * sizeof(struct parameter));
    }

    p_parameters->list[p_parameters->length] = *p_parameter;
    p_parameters->length += 1;
}

struct route parse_route(const char* p_format) {
    char* tokenizer;
    struct string format = new_string(p_format);

    struct route route = {0};

    char* part_str = strtok_r(format.data, "/", &tokenizer);
    while (part_str != NULL) {
        const size_t part_str_last_index = strlen(part_str) - 1;

        struct route_part part = {0};

        if (part_str[0] == '{' && part_str[part_str_last_index] == '}') {
            part.type = ROUTE_PART_TYPE_PARAMETER;
            part_str++;
            part_str[part_str_last_index - 1] = 0;
        } else {
            part.type = ROUTE_PART_TYPE_CONSTANT;
        }

        part.name = new_string(part_str);

        append_route_parts(&route, &part);
        part_str = strtok_r(NULL, "/", &tokenizer);
    }

    free_string(&format);
    return route;
}

bool match_route(const char* p_path, const struct route* p_route, struct parameters* p_parameters) {
    struct string path = new_string(p_path);

    char* tokenizer = NULL;
    char* path_part = strtok_r(path.data, "/", &tokenizer);

    for (const struct route_part* part = p_route->parts; part < p_route->parts + p_route->parts_length; part++) {
        if (path_part == NULL) {
            return false;
        }

        if (part->type == ROUTE_PART_TYPE_CONSTANT) {
            if (strcmp(path_part, part->name.data) != 0) {
                return false;
            }
        } else if (part->type == ROUTE_PART_TYPE_PARAMETER) {
            struct parameter parameter = {0};
            parameter.name = new_string(part->name.data);
            parameter.value = new_string(path_part);

            append_parameters(p_parameters, &parameter);
        }

        path_part = strtok_r(NULL, "/", &tokenizer);
    }

    return true;
}
