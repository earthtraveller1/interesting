#include <stdio.h>

#include "common.h"

#include "http.h"

struct string serialize_http_response(const struct http_response* response) {
    struct string string = new_string("HTTP/1.1 ");
    string_append_literal(&string, response->status);
    string_append_literal(&string, "\r\nContent-Type: ");
    string_append_literal(&string, response->content_type);
    string_append_literal(&string, "\r\nContent-Length: ");

    char content_length_string[64] = {0};
    sprintf(content_length_string, "%zu", response->content_length);
    string_append_literal(&string, content_length_string);

    string_append_literal(&string, "\r\n\r\n");
    string_append_literal(&string, response->body);

    return string;
}
