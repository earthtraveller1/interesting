#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "baseserver.h"
#include "http.h"

#define PORT 6969

static struct baseserver global_server = {0};

void interrupt_handler(int signal) {
    (void)signal;

    fprintf(stderr, "Closing server...\n");
    destroy_baseserver(global_server);
    exit(0);
}

void on_connection(int client_socket) {
    fprintf(stderr, "Client connected: %d\n", client_socket);

    struct string request_string = {0};

    size_t read_amount = 0;
    char character;

    do {
        read_amount = recv(client_socket, &character, 1, 0);
        string_append_char(&request_string, character);
        const char* tail_of_request = string_get_tail(&request_string, 5);
        fprintf(stderr, "[debug]: tail of request: %s\n", tail_of_request);

        if (strcmp(tail_of_request, "\r\n\r\n") == 0) {
            break;
        }
    } while (read_amount > 0);

    fprintf(stderr, "[debug]: Done reading from socket\n");

    if (read_amount < 0) {
        fprintf(stderr, "ERROR: Failed to read from socket\n");
        close(client_socket);
        return;
    }

    const struct http_request request = parse_http_request(request_string.data);

    struct string body = new_string("<!DOCTYPE html><html><head><title>Hello, World!</title></head><body><h1>");
    string_concat(&body, &request.path);
    string_append_literal(&body, "</h1><p>Hello, World!</p></body></html>");

    const struct http_response response = {
        .status = "200 OK",
        .content_type = "text/html",
        .content_length = body.length,
        .body = body.data
    };

    const struct string response_string = serialize_http_response(&response);
    send(client_socket, response_string.data, response_string.length, 0);
    fprintf(stderr, "DEBUG:\n%s\n", response_string.data);
    close(client_socket);
}

int main(void) {
    const struct server_error server_error = create_baseserver(PORT, INADDR_ANY);
    if (server_error.error != ERROR_SUCCESS) {
        return 1;
    }

    global_server = server_error.server;
    global_server.on_connection = on_connection;

    signal(SIGINT, interrupt_handler);

    return run_baseserver(&global_server);
}
