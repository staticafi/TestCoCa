#include <stdint.h>

#define INT128_MAX (__int128)(((unsigned __int128) 0 - 1) >> 1)
#define INT128_MIN (-INT128_MAX - 1)

extern __int128 __VERIFIER_nondet_int128();

int main() {
  __int128 x = __VERIFIER_nondet_int128();
  if (x == INT128_MAX) {
    return 1;
  }
  if (x == INT128_MIN) {
    return 1;
  }
  if (x == 0) {
    return 1;
  }
  return 0;
}
