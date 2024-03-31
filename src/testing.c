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

    test_assert(template.children_length == 3);
    test_assert(template.children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[0].text.data, "Hello, ") == 0);
    test_assert(template.children[1].type == TEMPLATE_VAR);
    test_assert(strcmp(template.children[1].var.data, "name") == 0);
    test_assert(template.children[2].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[2].text.data, "!") == 0);

    return true;
}

static bool template_parsing_test_for_if(void) {
    const char* template_str = 
        "%{{ if $neng_is_alive }}%\n" 
        "    Neng is alive!\n"
        "%{{ end }}%";

    const struct template template = parse_template(template_str);

    test_assert(template.children_length == 1);
    test_assert(template.children[0].type == TEMPLATE_IF);
    test_assert(strcmp(template.children[0].var.data, "neng_is_alive") == 0);
    test_assert(template.children[0].children_length == 1);
    test_assert(template.children[0].children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[0].children[0].text.data, "\n    Neng is alive!\n") == 0);

    return true;
}

static bool template_parsing_test_for_for(void) {
    const char* template_str = 
        "%{{ for $name in $names }}%\n"
        "    Hello, %{{ $name }}%, you are pretty cringe, not gonna lie.\n"
        "%{{ end }}%";

    const struct template template = parse_template(template_str);

    test_assert(template.children_length == 1);
    test_assert(template.children[0].type == TEMPLATE_FOR);
    test_assert(strcmp(template.children[0].var.data, "name") == 0);
    test_assert(strcmp(template.children[0].second_var.data, "names") == 0);

    test_assert(template.children[0].children_length == 3);
    test_assert(template.children[0].children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[0].children[0].text.data, "\n    Hello, ") == 0);
    test_assert(template.children[0].children[1].type == TEMPLATE_VAR);
    test_assert(strcmp(template.children[0].children[1].var.data, "name") == 0);
    test_assert(template.children[0].children[2].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[0].children[2].text.data, ", you are pretty cringe, not gonna lie.\n") == 0);

    return true;
}

static bool template_parsing_test_for_else(void) {
    const char* template_str = 
        "%{{ if $neng_is_alive }}%\n"
        "    Neng is alive!\n"
        "%{{ end }}%\n"
        "%{{ else }}%\n"
        "    Neng is dead!\n"
        "%{{ end }}%";

    const struct template template = parse_template(template_str);

    test_assert(template.children_length == 3);

    test_assert(template.children[0].type == TEMPLATE_IF);
    test_assert(strcmp(template.children[0].var.data, "neng_is_alive") == 0);
    test_assert(template.children[0].children_length == 1);
    test_assert(template.children[0].children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[0].children[0].text.data, "\n    Neng is alive!\n") == 0);

    test_assert(template.children[1].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[1].text.data, "\n") == 0);

    test_assert(template.children[2].type == TEMPLATE_ELSE);
    test_assert(template.children[2].children_length == 1);
    test_assert(template.children[2].children[0].type == TEMPLATE_TEXT);
    test_assert(strcmp(template.children[2].children[0].text.data, "\n    Neng is dead!\n") == 0);
    
    return true;
}

static bool template_rendering_test(void) {
    const char* template_str = 
        "Hello, %{{ $name }}%, you are pretty cringe, not gonna lie.";

    const struct template template = parse_template(template_str);

    struct template_parameters parameters = {0};
    append_template_parameter(&parameters, &(struct template_parameter) {
        .name = new_string("name"),
        .type = TEMPLATE_PARAMETER_TEXT,
        .text = new_string("Neng Li"),
    });

    const struct string result = render_template(&template, &parameters);
    test_assert(strcmp(result.data, "Hello, Neng Li, you are pretty cringe, not gonna lie.") == 0);

    free_string(&result);

    return true;
}

int run_tests(void) {
    run_test(http_parser_test);
    run_test(router_test);
    run_test(route_matching_test);
    run_test(template_parsing_test);
    run_test(template_parsing_test_for_if);
    run_test(template_parsing_test_for_for);
    run_test(template_parsing_test_for_else);
    run_test(template_rendering_test);

    return 0;
}

#endif
