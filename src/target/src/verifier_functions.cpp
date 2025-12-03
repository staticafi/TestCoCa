#include <boost/lexical_cast.hpp>
#include <target/target.hpp>
#include <utility/config.hpp>
#include <stdbool.h>

static_assert(sizeof(bool) == 1, "sizeof(bool) != 1");

static_assert(sizeof(char) == 1, "sizeof(char) != 1");
static_assert(sizeof(short) == 2, "sizeof(short) != 2");
static_assert(sizeof(int) == 4, "sizeof(int) != 4");
static_assert(sizeof(long) == 4 || sizeof(long) == 8, "sizeof(long) != 4 && sizeof(long) != 8");

static_assert(sizeof(unsigned char) == 1, "sizeof(unsigned char) != 1");
static_assert(sizeof(unsigned short) == 2, "sizeof(unsigned short) != 2");
static_assert(sizeof(unsigned int) == 4, "sizeof(unsigned int) != 4");
static_assert(sizeof(unsigned long) == 4 || sizeof(unsigned long) == 8, "sizeof(unsigned long) != 4 && sizeof(unsigned long) != 8");

static_assert(sizeof(float) == 4, "sizeof(float) != 4");
static_assert(sizeof(double) == 8, "sizeof(double) != 8");

using namespace instrumentation;

extern "C" {
char __VERIFIER_nondet_char() {
    char n;
    target->on_read<char>(&n);
    return n;
}

unsigned char __VERIFIER_nondet_uchar() {
    unsigned char n;
    target->on_read(&n);
    return n;
}

_Bool __VERIFIER_nondet_bool() {
    char n;
    target->on_read(&n);
    if (n > 0) {
        return true;
    }
    return false;
}

short __VERIFIER_nondet_short() {
    short n;
    target->on_read(&n);
    return n;
}

unsigned short __VERIFIER_nondet_ushort() {
    unsigned short n;
    target->on_read(&n);
    return n;
}

int __VERIFIER_nondet_int() {
    int n;
    target->on_read(&n);
    return n;
}

unsigned int __VERIFIER_nondet_uint() {
    unsigned int n;
    target->on_read(&n);
    return n;
}

long __VERIFIER_nondet_long() {
    long n;
    target->on_read(&n);
    return n;
}

unsigned long __VERIFIER_nondet_ulong() {
    unsigned long n;
    target->on_read(&n);
    return n;
}

long long int __VERIFIER_nondet_longlong() {
    long long int n;
    target->on_read(&n);
    return n;
}

unsigned long long int __VERIFIER_nondet_ulonglong() {
    unsigned long long int n;
    target->on_read(&n);
    return n;
}

size_t __VERIFIER_nondet_size_t() {
    size_t n;
    target->on_read(&n);
    return n;
}

#if CPU_TYPE() == CPU64()
    __int128 __VERIFIER_nondet_int128() {
        __int128 n;
        target->on_read(&n);
        return n;
    }

    unsigned __int128 __VERIFIER_nondet_uint128() {
        unsigned __int128 n;
        target->on_read(&n);
        return n;
    }
#endif

float __VERIFIER_nondet_float() {
    float n;
    target->on_read(&n);
    return n;
}

double __VERIFIER_nondet_double() {
    double n;
    target->on_read(&n);
    return n;
}

void __VERIFIER_nondet_memory(void* mem, size_t size) {
    auto *p = (unsigned char*) mem;

    for (int i = 0; i < size; ++i) {
        p[i] = __VERIFIER_nondet_uchar();
    }
}

// aliases --------------

unsigned char __VERIFIER_nondet_u8() { return __VERIFIER_nondet_uchar(); }
unsigned char __VERIFIER_nondet_unsigned_char() { return __VERIFIER_nondet_uchar(); }

unsigned short __VERIFIER_nondet_u16() { return __VERIFIER_nondet_ushort(); }
unsigned short __VERIFIER_nondet_unsigned_short() { return __VERIFIER_nondet_ushort(); }

unsigned int __VERIFIER_nondet_u32() { return __VERIFIER_nondet_uint(); }
unsigned int __VERIFIER_nondet_unsigned_int() { return __VERIFIER_nondet_uint(); }

}
