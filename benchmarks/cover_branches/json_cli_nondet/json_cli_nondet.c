#include <string.h>

extern int __VERIFIER_nondet_int(void);

static int has_flag(int argc, char** argv, const char* flag)
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], flag) == 0)
            return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (has_flag(argc, argv, "--skip")) {
        return 0;
    }

    int x = __VERIFIER_nondet_int();

    if (has_flag(argc, argv, "--strict")) {
        if (x > 0)
            return 1;
    } else {
        if (x < 0)
            return 2;
    }

    return 0;
}
