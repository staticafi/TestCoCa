#include <stdbool.h>

extern short __VERIFIER_nondet_bool();

int main() {
    bool x = __VERIFIER_nondet_bool();

    if (x && !x) return 1;

    return 0;
}