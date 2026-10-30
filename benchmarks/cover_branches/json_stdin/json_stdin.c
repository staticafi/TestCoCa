#include <stdio.h>

int main(void)
{
    int x = 0;

    if (scanf("%d", &x) != 1) {
        return 1;
    }

    if (x > 0) {
        return 2;
    }

    if (x < 0) {
        return 3;
    }

    return 0;
}
