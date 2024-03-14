#include <config.h>
#include <string.h>

#include "http.h"
#include "router.h"

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

int run_tests(void) {
    run_test("http_parser_test", http_parser_test);
    run_test("router_test", router_test);

    return 0;
}

#endif
