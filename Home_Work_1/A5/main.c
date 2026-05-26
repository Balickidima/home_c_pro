#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t N;
    if (scanf("%u", &N) != 1) {
        return 1;
    }

    int count = 0;
    while (N) {
        N &= (N - 1);  // Brian Kernighan's algorithm
        count++;
    }

    printf("%d\n", count);
    return 0;
}
