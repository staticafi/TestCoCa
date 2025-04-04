#include <stdbool.h>

void reach_error() { return; }

extern short __VERIFIER_nondet_bool();

int main() {
    bool x = __VERIFIER_nondet_bool();

    if (x && !x) reach_error();

    return 0;
}