#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#ifdef _WIN32
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <config.h>

#include "common.h"
#include "http.h"

#ifdef INTERESTING_BUILD_TESTS
#include "testing.h"
#endif

#define PORT 6969

static struct http_server global_server;
static struct string global_404_page;

static void interrupt_handler(int signal) {
    (void)signal;

    fprintf(stderr, "Closing server...\n");
    free_http_server(&global_server);
    exit(0);
}

static struct http_response on_request(const struct http_request* request) {
    struct http_response response = {0};

    struct string page_path = new_string("pages");
    if (strcmp(request->path.data, "/") == 0) {
        string_append_literal(&page_path, "/index.html");
    } else {
        string_concat(&page_path, &request->path);
    }

    const struct string_error page_content = read_string_from_file(page_path.data);
    if (page_content.error != ERROR_SUCCESS) {
        // Creates a new copy of the 404 page, because we don't want the original one to be freed!
        const struct string page_404 = new_string(global_404_page.data);

        response.status = "404 Not Found";
        response.content_type = "text/html";
        response.content_length = page_404.length;
        response.body = page_404;

        return response;
    }

    response.status = "200 OK";
    response.content_type = "text/html";
    response.content_length = page_content.string.length;
    response.body = page_content.string;

    free_string(&page_path);

    return response;
}

int main(int argc, char** argv) {
#ifdef INTERESTING_BUILD_TESTS
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        return run_tests();
    }
#endif
    struct http_server_error http_server = create_http_server(PORT, 0);
    if (http_server.error != ERROR_SUCCESS) {
        return 1;
    }

    const struct string_error page_404 = read_string_from_file("pages/404.html");
    if (page_404.error != ERROR_SUCCESS) {
        return 1;
    }

    global_server = http_server.server;
    global_404_page = page_404.string;

    global_server.on_request = on_request;
    
    signal(SIGINT, interrupt_handler);

    return run_http_server(&global_server);
}
