#ifndef INCLUDED_COMMON_H
#define INCLUDED_COMMON_H

#include <stddef.h>

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

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define string_get_tail(string, from_index) ((string)->data + max((string)->length - (from_index), 0))

void free_string(const struct string* string);

#endif
