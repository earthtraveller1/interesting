#ifndef INCLUDED_BASESERVER_H
#define INCLUDED_BASESERVER_H

#include <stdint.h>

#include "common.h"

typedef void (*on_connection_t)(int client_socket);

struct baseserver {
    int socket;
    on_connection_t on_connection;
};

struct server_error {
    struct baseserver server;
    enum error error;
};

struct server_error create_baseserver(uint16_t port, uint32_t p_address); 
enum error run_baseserver(const struct baseserver* p_server);
void destroy_baseserver(struct baseserver p_server); 

#endif
