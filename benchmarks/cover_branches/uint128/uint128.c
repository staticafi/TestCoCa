#include <stdint.h>

extern __int128 __VERIFIER_nondet_uint128();
const unsigned __int128 UINT128_MAX = (__uint128_t)0 - 1;

int main() {
  unsigned __int128 x = __VERIFIER_nondet_uint128();
  if (x == UINT128_MAX) {
    return 1;
  }
  if (x == 10000) {
    return 1;
  }
  if (x == 0) {
    return 1;
  }
  return 0;
}
