#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <config.h>

#include "common.h"
#include "http.h"

#ifdef INTERESTING_BUILD_TESTS
#include "testing.h"
#endif

#define PORT 6969

static struct http_server global_server;

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

    FILE* page_file = fopen(page_path.data, "r");
    if (page_file == NULL) {
        response.status = "500 Internal Server Error";
        response.content_type = "text/plain";
        response.content_length = 0;

        fprintf(stderr, "[ERROR]: Failed to open %s\n", page_path.data);

        return response;
    }

    char character;
    struct string page_content = {0};

    do {
        character = fgetc(page_file);
        string_append_char(&page_content, character);
    } while (character != EOF);

    response.status = "200 OK";
    response.content_type = "text/html";
    response.content_length = page_content.length;
    response.body = page_content;

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

    global_server = http_server.server;
    global_server.on_request = on_request;
    
    signal(SIGINT, interrupt_handler);

    return run_http_server(&global_server);
}
