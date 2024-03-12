#ifndef INCLUDED_HTTP_H
#define INCLUDED_HTTP_H

#include <stddef.h>

#include "common.h"
#include "baseserver.h"

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

    struct http_headers headers;
};

struct http_response {
    const char* status;
    const char* content_type;
    size_t content_length;
    const char* body;
};

typedef struct http_response (*request_handler)(const struct http_request* request);

struct http_server {
    struct baseserver base_server;
    request_handler on_request;
};

struct http_server_error {
    struct http_server server;
    enum error error;
};

struct http_header new_http_header(const char* name, const char* value);

struct string serialize_http_response(const struct http_response* response);

struct http_request parse_http_request(const char* request_str);

// Note, the ownership of header is transferred to the headers object after this call
// Do not free the header after this call, as free_http_headers will do it for you
void http_headers_append(struct http_headers* headers, const struct http_header* header);

struct http_server_error create_http_server(uint16_t port, uint32_t p_address);

enum error run_http_server(struct http_server* server);

void free_http_server(struct http_server* server);

void free_http_request(const struct http_request* request);

void free_http_header(const struct http_header* header);

void free_http_headers(const struct http_headers* headers);

#endif
