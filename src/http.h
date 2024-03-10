#ifndef INCLUDED_HTTP_H
#define INCLUDED_HTTP_H

#include <stddef.h>

#include "common.h"

struct http_header {
    struct string name;
    struct string value;
};

struct http_headers {
    struct http_header* headers;
    size_t length;
    size_t capacity;
};

struct http_request {
    struct string method;
    struct string path;
};

struct http_response {
    const char* status;
    const char* content_type;
    size_t content_length;
    const char* body;
};

struct http_header new_http_header(const char* name, const char* value);

struct string serialize_http_response(const struct http_response* response);

struct http_request parse_http_request(const char* request_str);

void http_headers_append(struct http_headers* headers, const struct http_header* header);

void free_http_request(const struct http_request* request);

void free_http_header(const struct http_header* header);

void free_http_headers(const struct http_headers* headers);

#endif
