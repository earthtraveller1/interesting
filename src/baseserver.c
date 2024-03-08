#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "common.h"

#include "baseserver.h"

struct server_error create_baseserver(uint16_t port, uint32_t p_address) {
    struct server_error server = {0};

    server.server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server.server.socket < 0) {
        fprintf(stderr, "ERROR: Failed to create socket\n");
        server.error = ERROR_SOCKET_CREATION_FAILED;
        return server;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(p_address);
    address.sin_port = htons(port);

    if (bind(server.server.socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        fprintf(stderr, "ERROR: Failed to bind socket\n");
        server.error = ERROR_SOCKET_BIND_FAILED;
        return server;
    }

    return server;
}

enum error run_baseserver(const struct baseserver* p_server) {
    if (listen(p_server->socket, SOMAXCONN) < 0) {
        fprintf(stderr, "ERROR: Failed to listen on socket\n");
        return ERROR_SOCKET_LISTEN_FAILED;
    }

    while (true) {
        int client_socket = accept(p_server->socket, NULL, NULL);
        if (client_socket < 0) {
            continue;
        }

        p_server->on_connection(client_socket);

        close(client_socket);
    }
}

void destroy_baseserver(struct baseserver p_server) {
    close(p_server.socket);
}

