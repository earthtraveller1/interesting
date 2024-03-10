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

void http_headers_append(struct http_headers* p_headers, const struct http_header* p_header) {
    if (p_headers->capacity == 0) {
        p_headers->capacity = 16;
        p_headers->length = 0; // This is for safety, in case length isn't 0 for some reasons.
        p_headers->headers = malloc(sizeof(struct http_header) * p_headers->capacity);
    }

    if (p_headers->length == p_headers->capacity) {
        p_headers->capacity *= 2;
        p_headers->headers = realloc(p_headers->headers, sizeof(struct http_header) * p_headers->capacity);
    }

    p_headers->headers[p_headers->length] = *p_header;
    p_headers->length++;
}

void free_http_request(const struct http_request* request) {
    free_string(&request->method);
    free_string(&request->path);
}

void free_http_header(const struct http_header* header) {
    free_string(&header->name);
    free_string(&header->value);
}

void free_http_headers(const struct http_headers* headers) {
    for (
        const struct http_header* header = headers->headers; 
        header < headers->headers + headers->length; 
        header++
    ) {
        free_http_header(header);
    }

    free(headers->headers);
}
