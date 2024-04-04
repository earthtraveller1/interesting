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
#include "templates.h"

#ifdef INTERESTING_BUILD_TESTS
#include "testing.h"
#endif

#define PORT 6969

struct application {
    struct http_server server;
    struct router router;
    struct template name_template;
};

static struct application* interrupt_handler_application = NULL;

static void interrupt_handler(int signal) {
    (void)signal;

    fprintf(stderr, "Closing server...\n");
    free_http_server(&interrupt_handler_application->server);
    free_router(&interrupt_handler_application->router);
    exit(0);
}

static struct http_response on_request(const struct http_request* request, void* user_data) {
    const struct application* application = (struct application*)user_data;
    return route_request(&application->router, request);
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

static struct http_response name_handler(const struct parameters* parameters, const struct http_request* request, void* user_data) {
    (void)request;

    const struct application* application = (const struct application*)user_data;

    struct template_parameters template_parameters = {0};
    append_template_parameter(&template_parameters, &(struct template_parameter) {
        .name = new_string("name"),
        .type = TEMPLATE_PARAMETER_TEXT,
        .text = new_string(get_parameter(parameters, "name")->data),
    });

    const struct string result = render_template(&application->name_template, &template_parameters);

    free_template_parameters(&template_parameters);

    return (struct http_response) {
        .status = "200 OK",
        .content_type = "text/html",
        .content_length = result.length - 1,
        .body = result,
    };
}

int main(int argc, char** argv) {
#ifdef INTERESTING_BUILD_TESTS
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        return run_tests();
    }
#endif
    struct application application = {0};

    struct http_server_error http_server = create_http_server(PORT, 0);
    if (http_server.error != INTERESTING_ERROR_SUCCESS) {
        return 1;
    }

    application.server = http_server.server;
    application.server.on_request = on_request;
    application.server.user_data = &application;

    add_route_handler(&application.router, "/", index_handler);
    add_route_handler(&application.router, "/neng", neng_handler);
    add_route_handler(&application.router, "/name/{name}", name_handler);

    application.router.user_data = &application;

    const struct template_error name_template = parse_template_from_file("pages/name.html");
    if (name_template.error != INTERESTING_ERROR_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to parse template\n");
        return 1;
    }
    
    application.name_template = name_template.template;
    
    interrupt_handler_application = &application;
    signal(SIGINT, interrupt_handler);

    return run_http_server(&application.server);
}
