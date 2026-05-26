#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t N;
    unsigned int K;
    if (scanf("%u %u", &N, &K) != 2) {
        return 1;
    }

    uint32_t result = (N >> K) | (N << (32 - K));
    printf("%u\n", result);
    return 0;
}
