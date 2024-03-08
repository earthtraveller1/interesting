#ifndef INCLUDED_HTTP_H
#define INCLUDED_HTTP_H

#include <stddef.h>

struct http_response {
    const char* status;
    const char* content_type;
    size_t content_length;
    const char* body;
};

struct string serialize_http_response(const struct http_response* response);

#endif
