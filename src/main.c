#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#ifdef _WIN32
#else
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <unistd.h>
#endif

#include <config.h>

#include "common.h"
#include "http.h"
#include "router.h"

#ifdef INTERESTING_BUILD_TESTS
#include "testing.h"
#endif

#define PORT 6969

static struct http_server global_server;
static struct router global_router;

static struct string global_404_page;

static void interrupt_handler(int signal) {
    (void)signal;

    fprintf(stderr, "Closing server...\n");
    free_http_server(&global_server);
    free_router(&global_router);
    exit(0);
}

static struct http_response on_request(const struct http_request* request, void* user_data) {
    const struct router* router = (const struct router*)user_data;
    return route_request(router, request);
}

static struct http_response index_handler(const struct parameters* parameters, const struct http_request* request, void* user_data) {
    (void)parameters;
    (void)request;
    (void)user_data;

    const struct string_error page = read_string_from_file("pages/index.html");

    return (struct http_response) {
        .status = "200 OK",
        .content_type = "text/html",
        .content_length = page.string.length - 1,
        .body = page.string,
    };
}

static struct http_response neng_handler(const struct parameters* parameters, const struct http_request* request, void* user_data) {
    (void)parameters;
    (void)request;
    (void)user_data;

    const struct string_error page = read_string_from_file("pages/neng.html");

    return (struct http_response) {
        .status = "200 OK",
        .content_type = "text/html",
        .content_length = page.string.length - 1,
        .body = page.string,
    };
}

int main(int argc, char** argv) {
#ifdef INTERESTING_BUILD_TESTS
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        return run_tests();
    }
#endif
    struct http_server_error http_server = create_http_server(PORT, 0);
    if (http_server.error != INTERESTING_ERROR_SUCCESS) {
        return 1;
    }

    add_route_handler(&global_router, "/", index_handler);
    add_route_handler(&global_router, "/neng", neng_handler);

    const struct string_error page_404 = read_string_from_file("pages/404.html");
    if (page_404.error != INTERESTING_ERROR_SUCCESS) {
        return 1;
    }

    global_server = http_server.server;
    global_404_page = page_404.string;

    global_server.on_request = on_request;
    global_server.user_data = &global_router;
    
    signal(SIGINT, interrupt_handler);

    return run_http_server(&global_server);
}
