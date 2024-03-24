#include <config.h>
#include <string.h>

#include "http.h"
#include "router.h"
#include "templates.h"

#ifdef INTERESTING_BUILD_TESTS

#include "testing.h"

static bool http_parser_test(void) {
    const char* request_str =
        "GET /bozo/ratio HTTP/1.1\r\n"
        "Host: www.ccp.org\r\n"
        "Accept-Language: en-US\r\n"
        "\r\n";

    const struct http_request request = parse_http_request(request_str);
    
    test_assert(strcmp(request.method.data, "GET") == 0);
    test_assert(strcmp(request.path.data, "/bozo/ratio") == 0);
    test_assert(request.headers.length == 2);
    test_assert(strcmp(request.headers.headers[0].name.data, "Host") == 0);
    test_assert(strcmp(request.headers.headers[0].value.data, "www.ccp.org") == 0);
    test_assert(strcmp(request.headers.headers[1].name.data, "Accept-Language") == 0);
    test_assert(strcmp(request.headers.headers[1].value.data, "en-US") == 0);

    return true;
}

static bool router_test(void) {
    const char* route_str = "/hello/world/{seymour}";

    const struct route route = parse_route(route_str);

    test_assert(route.parts_length == 3);
    test_assert(strcmp(route.parts[0].name.data, "hello") == 0);
    test_assert(route.parts[0].type == ROUTE_PART_TYPE_CONSTANT);
    test_assert(strcmp(route.parts[1].name.data, "world") == 0);
    test_assert(route.parts[1].type == ROUTE_PART_TYPE_CONSTANT);
    test_assert(strcmp(route.parts[2].name.data, "seymour") == 0);
    test_assert(route.parts[2].type == ROUTE_PART_TYPE_PARAMETER);

    return true;
}

static bool route_matching_test(void) {
    struct parameters parameters = {0};
    const struct route route = parse_route("/neng/li/{school}");

    test_assert(match_route("/neng/li/oakville", &route, &parameters) == true);
    test_assert(parameters.length == 1);
    test_assert(strcmp(parameters.list[0].name.data, "school") == 0);
    test_assert(strcmp(parameters.list[0].value.data, "oakville") == 0);

    return true;
}

static bool template_parsing_test(void) {
    const char* template_str = "Hello, %{{ $name }}%!";
    const struct template template = parse_template(template_str);

    test_assert(template.children_length == 2);
    test_assert(template.children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[0].text.data, "Hello, ") == 0);
    test_assert(template.children[1].type == TEMPLATE_VAR);
    test_assert(strcmp(template.children[1].var.data, "name") == 0);

    return true;
}

static bool template_parsing_test_for_if(void) {
    const char* template_str = "%{{ if $neng_is_alive }}% Neng is alive! %{{ end }}%";
    const struct template template = parse_template(template_str);

    test_assert(template.children_length == 2);
    test_assert(template.children[0].type == TEMPLATE_TEXT);
    test_assert(template.children[0].text.data == NULL);

    test_assert(template.children[1].type == TEMPLATE_IF);
    test_assert(strcmp(template.children[1].var.data, "neng_is_alive") == 0);
    test_assert(template.children[1].children_length == 1);
    test_assert(template.children[1].children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[1].children[0].text.data, " Neng is alive! ") == 0);

    return true;
}

int run_tests(void) {
    run_test(http_parser_test);
    run_test(router_test);
    run_test(route_matching_test);
    run_test(template_parsing_test);
    run_test(template_parsing_test_for_if);

    return 0;
}

#endif
