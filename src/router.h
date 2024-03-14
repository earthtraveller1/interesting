#ifndef INCLUDED_ROUTER_H
#define INCLUDED_ROUTER_H

#include "common.h"

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

struct route parse_route(const char* format);

bool match_route(const char* path, const struct route* route, struct parameters* parameters);

#endif
