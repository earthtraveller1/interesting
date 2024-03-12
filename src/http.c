#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "common.h"

#include "http.h"

static void on_connection(int client_socket, void* user_pointer);

struct http_header new_http_header(const char* name, const char* value) {
    return (struct http_header) {
        .name = new_string(name),
        .value = new_string(value),
    };
}

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
    string_append_literal(&string, response->body.data);

    return string;
}

struct http_request parse_http_request(const char* p_request_str) {
    struct http_request request = {0};

    char* request_str = malloc(sizeof(char) * (strlen(p_request_str) + 1));
    strcpy(request_str, p_request_str);

    char* tokenizer = NULL;
    char* first_line = strtok_r(request_str, "\r\n", &tokenizer);

    char* line_tokenizer = NULL;
    char* method = strtok_r(first_line, " ", &line_tokenizer);
    char* path = strtok_r(NULL, " ", &line_tokenizer);

    char* line;
    while ((line = strtok_r(NULL, "\r\n", &tokenizer)) != NULL) {
        char* name = strtok_r(line, ": ", &line_tokenizer);
        char* value = strtok_r(NULL, ": ", &line_tokenizer);

        struct http_header http_header = new_http_header(name, value);
        http_headers_append(&request.headers, &http_header);
    }

    request.method = new_string(method);
    request.path = new_string(path);

    free(request_str);

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

struct http_server_error create_http_server(uint16_t p_port, uint32_t p_address) {
    struct http_server_error server = {0};

    const struct server_error base_server = create_baseserver(p_port, p_address);
    if (base_server.error != ERROR_SUCCESS) {
        server.error = base_server.error;
        return server;
    }

    server.server.base_server = base_server.server;
    server.server.base_server.on_connection = on_connection;
    return server;
}

enum error run_http_server(struct http_server* p_server) {
    p_server->base_server.user_pointer = p_server;

    enum error error = run_baseserver(&p_server->base_server);
    if (error != ERROR_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to run server\n");
    }

    return error;
}

void free_http_server(struct http_server* p_server) {
    destroy_baseserver(p_server->base_server);
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

static void on_connection(int client_socket, void* user_pointer) {
    struct http_server* server = (struct http_server*)user_pointer;

    struct string request_string = {0};

    while (!string_ends_with(&request_string, "\r\n\r\n")) {
        char character;
        recv(client_socket, &character, 1, 0);
        string_append_char(&request_string, character);
    }

    const struct http_request http_request = parse_http_request(request_string.data);

    struct http_response response = {0};

    if (server->on_request != NULL) {
        response = server->on_request(&http_request);
    } else {
        response.status = "200 OK";
        response.content_type = "text/html";
        response.body = new_string(
            "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<title>Request Handler not Set</title>"
            "</head>"
            "<body>"
            "<h1>Request Handler not set</h1>"
            "<p>The request handler for the HTTP server has not been set.</p>"
            "</body>"
            "</html>"
        );
    }

    struct string response_string = serialize_http_response(&response);
    send(client_socket, response_string.data, response_string.length, 0);

    free_string(&response.body);
    free_string(&response_string);
    free_string(&request_string);
    free_http_request(&http_request);
}
