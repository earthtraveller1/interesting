#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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

    const char* body = "Hello, World!";

    const struct http_response response = {
        .status = "200 OK",
        .content_type = "text/plain",
        .content_length = strlen(body),
        .body = body
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
