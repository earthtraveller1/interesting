#include <string.h>
#include <stdlib.h>

#include "common.h"

struct string new_string(const char* p_literal) {
    struct string string;
    memset(&string, 0, sizeof(string));

    string.length = strlen(p_literal) + 1;
    string.capacity = string.length;
    string.data = malloc(string.capacity);

    memset(string.data, 0, string.capacity);
    memcpy(string.data, p_literal, string.length);

    return string;
}

void string_append_char(struct string* p_string, char p_char) {
    if (p_string->length == p_string->capacity) {
        p_string->capacity *= 2;
        p_string->data = realloc(p_string->data, p_string->capacity);
    }

    p_string->data[p_string->length - 1] = p_char;
    p_string->data[p_string->length] = 0;
    p_string->length += 1;
}

void string_append_literal(struct string* p_string, const char* p_literal) {
    for (const char* c = p_literal; *c; c++) {
        string_append_char(p_string, *c);
    }
}

void string_concat(struct string* p_string, const struct string* p_other) {
    string_append_literal(p_string, p_other->data);
}

void free_string(const struct string* p_string) {
    free(p_string->data);
}
