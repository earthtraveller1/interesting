#ifndef INCLUDED_TESTING_H
#define INCLUDED_TESTING_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define test_assert(expr)\
    do {\
        if (!(expr)) {\
            fprintf(stderr, "\nassertion failed at " __FILE__ ":%d", __LINE__);\
            return false;\
        }\
    } while (0)

#define run_test(name, test)\
    do {\
        fprintf(stderr, "running test " name "\t\t\t.........");\
        if (!test()) {\
            fprintf(stderr, "\n\033[91m" name " failed\033[0m\n");\
            return EXIT_FAILURE;\
        } else {\
            fprintf(stderr, "\033[92mpassed\033[0m\n");\
        }\
    } while (0)

int run_tests(void);

#endif
