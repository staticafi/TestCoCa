extern int __VERIFIER_nondet_int(void);

int main(void)
{
    int x = __VERIFIER_nondet_int();

    while (x > 0 && x < 100) {
        if (x == 42)
            break;
        x++;
    }

    return 0;
}
