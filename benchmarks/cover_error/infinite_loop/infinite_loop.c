#include <stdbool.h>

extern short __VERIFIER_nondet_short();
extern bool __VERIFIER_nondet_bool();

void reach_error() { return; }

int main() {
    short x = __VERIFIER_nondet_short();
    bool b = __VERIFIER_nondet_bool();

    reach_error();

    while (x) {
      b = !b;
    }

    return 0;
}