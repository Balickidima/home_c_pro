#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t N;
    unsigned int K;
    if (scanf("%u %u", &N, &K) != 2) {
        return 1;
    }

    if (K >= 32) {
        printf("%u\n", N);
        return 0;
    }

    uint32_t max_val = 0;
    for (unsigned int i = 0; i <= 32 - K; i++) {
        uint32_t val = (N >> i) & ((1U << K) - 1);
        if (val > max_val) {
            max_val = val;
        }
    }

    printf("%u\n", max_val);
    return 0;
}
