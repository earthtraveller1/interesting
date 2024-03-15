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

    if (p_route->parts_length == 0) {
        free_string(&path);
        return strcmp(p_path, "/") == 0;
    }

    for (const struct route_part* part = p_route->parts; part < p_route->parts + p_route->parts_length; part++) {
        if (path_part == NULL) {
            free_string(&path);
            return false;
        }

        if (part->type == ROUTE_PART_TYPE_CONSTANT) {
            if (strcmp(path_part, part->name.data) != 0) {
                free_string(&path);
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

    free_string(&path);
    return true;
}

const struct string* get_parameter(const struct parameters* parameters, const char* name) {
    for (const struct parameter* parameter = parameters->list; parameter < parameters->list + parameters->length; parameter++) {
        if (strcmp(parameter->name.data, name) == 0) {
            return &parameter->value;
        }
    }

    return NULL;
}

void add_route_handler(struct router* p_router, const char* p_route, route_handler_proc_t p_proc) {
    if (p_router->handlers_capacity == 0) {
        p_router->handlers_length = 0;
        p_router->handlers_capacity = 1;
        p_router->handlers = malloc(sizeof(struct route_handler));
    }

    if (p_router->handlers_capacity == p_router->handlers_length) {
        p_router->handlers_capacity *= 2;
        p_router->handlers = realloc(p_router->handlers, p_router->handlers_capacity * sizeof(struct route_handler));
    }

    const struct route route = parse_route(p_route);

    p_router->handlers[p_router->handlers_length].route = route;
    p_router->handlers[p_router->handlers_length].proc = p_proc;
    p_router->handlers_length += 1;
}

struct http_response route_request(const struct router* router, const struct http_request* request) {
    for (const struct route_handler* handler = router->handlers; handler < router->handlers + router->handlers_length; handler++) {
        struct parameters parameters = {0};
        if (match_route(request->path.data, &handler->route, &parameters)) {
            return handler->proc(&parameters, request, router->user_data);
        }

        free_parameters(&parameters);
    }

    return (struct http_response) {
        .status = "404 Not Found",
        .content_type = "text/html",
        .content_length = 0,
        .body = (struct string) {0},
    };
}

void free_route_part(const struct route_part* part) {
    free_string(&part->name);
}

void free_route(const struct route* route) {
    for (const struct route_part* part = route->parts; part < route->parts + route->parts_length; part++) {
        free_route_part(part);
    }
}

void free_parameter(const struct parameter* parameter) {
    free_string(&parameter->name);
    free_string(&parameter->value);
}

void free_parameters(const struct parameters* parameters) {
    for (const struct parameter* parameter = parameters->list; parameter < parameters->list + parameters->length; parameter++) {
        free_parameter(parameter);
    }

    free(parameters->list);
}

void free_route_handler(const struct route_handler* handler) {
    free_route(&handler->route);
}

void free_router(const struct router* router) {
    for (const struct route_handler* handler = router->handlers; handler < router->handlers + router->handlers_length; handler++) {
        free_route_handler(handler);
    }

    free(router->handlers);
}

