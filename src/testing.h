#ifndef INCLUDED_TESTING_H
#define INCLUDED_TESTING_H

#define STRINGIFY(x) #x
#define test_assert(expr)\
    do {\
        if (!(expr)) {\
            fprintf(stderr, "assertion failed at " __FILE__ ":" STRINGIFY(__LINE__) "\n");\
            return false;\
        }\
    } while (0)

#endif
