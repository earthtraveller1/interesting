#ifndef INCLUDED_ROUTER_H
#define INCLUDED_ROUTER_H

#include "common.h"
#include "http.h"

struct route_part {
    enum type {
        ROUTE_PART_TYPE_CONSTANT,
        ROUTE_PART_TYPE_PARAMETER
    } type;

    struct string name;
};

struct route {
    struct route_part* parts;
    size_t parts_length;
    size_t parts_capacity;
};

struct parameter {
    struct string name;
    struct string value;
};

struct parameters {
    struct parameter* list;
    size_t length;
    size_t capacity;
};

typedef void (*route_handler_proc_t)(const struct parameters* parameters, const struct http_request* request, void* user_data);

struct route_handler {
    route_handler_proc_t proc;
    struct route route;
};

struct router {
    struct route_handler* handlers;
    size_t handlers_length;
    size_t handlers_capacity;
    void* user_data;
};

struct route parse_route(const char* format);

bool match_route(const char* path, const struct route* route, struct parameters* parameters);

// May return NULL, when the parameter does not exist
const struct string* get_parameter(const struct parameters* parameters, const char* name);

void add_route_handler(struct router* router, const char* route, route_handler_proc_t proc);

void route_request(const struct router* router, const struct http_request* request);

#endif
