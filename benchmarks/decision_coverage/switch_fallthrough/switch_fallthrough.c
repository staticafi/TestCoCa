extern int __VERIFIER_nondet_int(void);

int main(void)
{
    int y = __VERIFIER_nondet_int();
    int d = 0;

    switch (y) {
        case 1: d = 1;
        case 2: d = 2; break;
        default: d = 42; break;
    }

    return d;
}
