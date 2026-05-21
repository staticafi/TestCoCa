extern int __VERIFIER_nondet_int(void);
extern void* malloc(unsigned int size);

int main() {
    int N = __VERIFIER_nondet_int();
    if (N <= 0) return 1;

    long long *a = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++)
        a[i] = i;
    return 0;
}
