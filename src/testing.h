#ifndef INCLUDED_TESTING_H
#define INCLUDED_TESTING_H

#include <stdio.h>
#include <stdbool.h>

#define STRINGIFY(x) #x
#define test_assert(expr)\
    do {\
        if (!(expr)) {\
            fprintf(stderr, "assertion failed at " __FILE__ ":" STRINGIFY(__LINE__) "\n");\
            return false;\
        }\
    } while (0)

#define run_test(name, test)\
    do {\
        fprintf(stderr, "running test " name "\t\t\t.........");\
        if (!test()) {\
            fprintf(stderr, "\033[91mfailed\033[0m\n");\
        } else {\
            fprintf(stderr, "\033[92mpassed\033[0m\n");\
        }\
    } while (0)

void run_tests(void);

#endif
