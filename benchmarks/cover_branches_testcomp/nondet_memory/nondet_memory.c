#include <stdbool.h>
#include <stddef.h>

extern short __VERIFIER_nondet_memory(void*, size_t);

void reach_error()
{
    return;
}

typedef struct {
    _Bool b1;
    _Bool b2;
} bool_pair;

int main()
{
    bool_pair bp = {.b1 = 1, .b2 = 0};

    __VERIFIER_nondet_memory(&bp, sizeof(bool_pair));

    if (bp.b1 && bp.b2) {
        return 0;
    }

    if (!bp.b1 && !bp.b2) {
        return 1;
    }

    return 2;
}
