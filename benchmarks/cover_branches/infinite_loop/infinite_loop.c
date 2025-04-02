#include <stdbool.h>

extern short __VERIFIER_nondet_short();
extern bool __VERIFIER_nondet_bool();

int main() {
    short x = __VERIFIER_nondet_short();
    bool b = __VERIFIER_nondet_bool();

    while (x) {
      b = !b;
    }

    return 0;
}