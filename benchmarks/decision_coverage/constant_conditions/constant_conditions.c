extern int __VERIFIER_nondet_int(void);

#define ZERO 0
#define ONE 1

int main(void)
{
    int x = __VERIFIER_nondet_int();
    int r = 0;

    while (ZERO) {
        if (x > 0)
            r = 1;
    }

    if (ONE)
        r = 2;

    if (x > 0)
        r = 3;

    return r;
}
