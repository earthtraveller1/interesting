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

#define PORT 6969

static struct baseserver global_server = {0};

void interrupt_handler(int signal) {
    (void)signal;

    fprintf(stderr, "Closing server...\n");
    destroy_baseserver(global_server);
    exit(0);
}

void on_connection(int client_socket) {
    fprintf(stderr, "Got a connection!\n");

    const char* content = "Hello, World!";
    const size_t content_length = strlen(content);

    char content_length_str[21];
    sprintf(content_length_str, "%zu", content_length);

    struct string response = new_string("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ");
    string_append_literal(&response, content_length_str);
    string_append_literal(&response, "\r\n\r\n");
    string_append_literal(&response, content);
    string_append_literal(&response, "\r\n");

    fprintf(stderr, "[DEBUG]: Here is the response body: %s\n", response.data);

    write(client_socket, response.data, response.length);

    free_string(&response);
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
