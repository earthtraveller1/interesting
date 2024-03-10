#ifndef INCLUDED_HTTP_H
#define INCLUDED_HTTP_H

#include <stddef.h>

#include "common.h"

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

struct string serialize_http_response(const struct http_response* response);

struct http_request parse_http_request(const char* request_str);

void free_http_request(const struct http_request* request);

#endif
