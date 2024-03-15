#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "common.h"

#include "baseserver.h"

struct server_error create_baseserver(uint16_t port, uint32_t p_address) {
#ifdef _WIN32
    (void)p_address;

    struct server_error server = {0};

    struct WSAData wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        server.error = ERROR_WSA_STARTUP_FAILED;
        return server;
    }

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port_str[16];
    sprintf(port_str, "%d", port);

    struct addrinfo* address_info;
    result = getaddrinfo(NULL, port_str, &hints, &address_info);
    if (result != 0) {
        server.error = ERROR_GETADDRINFO_FAILED;
        WSACleanup();
        return server;
    }

    server.server.socket = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (server.server.socket == INVALID_SOCKET) {
        server.error = ERROR_SOCKET_CREATION_FAILED;
        freeaddrinfo(address_info);
        WSACleanup();
        return server;
    }

    freeaddrinfo(address_info);

    result = bind(server.server.socket, address_info->ai_addr, (int)address_info->ai_addrlen);
    if (result == SOCKET_ERROR) {
        server.error = ERROR_SOCKET_BIND_FAILED;
        closesocket(server.server.socket);
        WSACleanup();
        return server;
    }

    return server;
#else
    struct server_error server = {0};

    server.server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server.server.socket < 0) {
        fprintf(stderr, "ERROR: Failed to create socket\n");
        server.error = ERROR_SOCKET_CREATION_FAILED;
        return server;
    }

    setsockopt(server.server.socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

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
#endif
}

enum error run_baseserver(const struct baseserver* p_server) {
#ifdef _WIN32
    if (listen(p_server->socket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "ERROR: Failed to listen on socket\n");
        return ERROR_SOCKET_LISTEN_FAILED;
    }

    while (true) {
        SOCKET client_socket = accept(p_server->socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            continue;
        }

        p_server->on_connection(client_socket, p_server->user_pointer);

        closesocket(client_socket);
    }
#else
    if (listen(p_server->socket, SOMAXCONN) < 0) {
        fprintf(stderr, "ERROR: Failed to listen on socket\n");
        return ERROR_SOCKET_LISTEN_FAILED;
    }

    while (true) {
        int client_socket = accept(p_server->socket, NULL, NULL);
        if (client_socket < 0) {
            continue;
        }

        p_server->on_connection(client_socket, p_server->user_pointer);

        close(client_socket);
    }
#endif
}

void destroy_baseserver(struct baseserver p_server) {
#ifdef _WIN32
    closesocket(p_server.socket);
    WSACleanup();
#else
    close(p_server.socket);
#endif
}

