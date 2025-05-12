#include <math.h>

extern float __VERIFIER_nondet_float();
extern double __VERIFIER_nondet_double();

void reach_error() { return; }

int main() {
    float f = __VERIFIER_nondet_float();
    double d = __VERIFIER_nondet_double();

    if (isinf(f) && isnan(d)) {
        reach_error();
    }

    return 0;
}