#include <string.h>

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
    if (has_flag(argc, argv, "--alpha")) {
        return 1;
    }

    if (has_flag(argc, argv, "--beta")) {
        return 2;
    }

    return 0;
}
