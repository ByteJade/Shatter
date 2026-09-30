#include <stdio.h>

int main() {
    {
        int i;
        printf(" int:\n");
        for (int i = 0; i < 10; i++) {
            printf("i: %i", i);
        }
        printf("\n int reverse:\n");
        for (int i = 10; i > 0; i--) {
            printf("i: %i", i);
        }
    }
    {
        char i;
        printf("\n char:\n");
        for (char i = 0; i < 10; i++) {
            printf("i: %i", i);
        }
        printf("\n char reverse:\n");
        for (char i = 10; i > 0; i--) {
            printf("i: %i", i);
        }
        printf("\n");
    }
    return 0;
}