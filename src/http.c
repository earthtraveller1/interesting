#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct http_request parse_http_request(const char* p_request_str) {
    struct http_request request = {0};

    char* request_str = malloc(sizeof(char) * (strlen(p_request_str) + 1));
    strcpy(request_str, p_request_str);

    char* tokenizer = NULL;
    char* first_line = strtok_r(request_str, "\r\n", &tokenizer);

    char* method = strtok_r(first_line, " ", &tokenizer);
    char* path = strtok_r(NULL, " ", &tokenizer);

    request.method = new_string(method);
    request.path = new_string(path);

    return request;
}
