#ifndef INCLUDED_COMMON_H
#define INCLUDED_COMMON_H

#include <stddef.h>
#include <stdbool.h>

enum error {
    INTERESTING_ERROR_SUCCESS = 0,
    ERROR_SOCKET_CREATION_FAILED = 1,
    ERROR_SOCKET_BIND_FAILED = 2,
    ERROR_SOCKET_LISTEN_FAILED = 3,
    ERROR_FILE_OPEN_FAILED = 4,
    ERROR_WSA_STARTUP_FAILED = 5,
    ERROR_GETADDRINFO_FAILED = 6,
    ERROR_REQUIRED_PARAMETER_NOT_PROVIDED = 7,
    ERROR_PARAMETER_WRONG_TYPE = 8,
    ERROR_ELSE_MUST_FOLLOW_IF = 9,
};

#ifdef _WIN32
#define strtok_r strtok_s
#endif

struct string {
    char* data;
    size_t length;
    size_t capacity;
};

struct string_error {
    enum error error;
    struct string string;
};

struct string_list {
    struct string* strings;
    size_t length;
    size_t capacity;
};

struct string new_string(const char* literal);

struct string_error read_string_from_file(const char* filename);

void string_append_char(struct string* string, char c);

void string_append_literal(struct string* string, const char* literal);

void string_concat(struct string* string, const struct string* other);

bool string_ends_with(struct string* string, const char* suffix);

void string_list_append(struct string_list* list, const struct string string);

void free_string(const struct string* string);

void free_string_list(const struct string_list* list);

#endif
