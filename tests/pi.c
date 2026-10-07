#include <stdio.h>

int main() {
    int r[2800 + 1];
    int i, k;
    int b, d;
    int c = 0;

    for (i = 0; i < 2800; i++) {
        r[i] = 2000;
    }

    for (k = 42; k > 0; k -= 14) {
        d = 0;

        i = k;
        for (;;) {
            d += r[i] * 10000;
            printf("%i/", d);
            b = 2 * i - 1;
            printf("%i/", b);

            r[i] = d % b;
            d /= b;
            printf("%i/", d);
            i--;
            if (i == 0) break;
            d *= i;
            printf("%i ", d);
        }
        printf("%i, %i, %.4d\n", c, d, c + d / 10000);
        c = d % 10000;
    }

    return 0;
}