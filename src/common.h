#ifndef INCLUDED_COMMON_H
#define INCLUDED_COMMON_H

#include <stddef.h>
#include <stdbool.h>

enum error {
    ERROR_SUCCESS = 0,
    ERROR_SOCKET_CREATION_FAILED = 1,
    ERROR_SOCKET_BIND_FAILED = 2,
    ERROR_SOCKET_LISTEN_FAILED = 3
};

struct string {
    char* data;
    size_t length;
    size_t capacity;
};

struct string new_string(const char* literal);

void string_append_char(struct string* string, char c);

void string_append_literal(struct string* string, const char* literal);

void string_concat(struct string* string, const struct string* other);

bool string_ends_with(struct string* string, const char* suffix);

void free_string(const struct string* string);

#endif
