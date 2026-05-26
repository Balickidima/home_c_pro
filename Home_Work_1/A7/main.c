#include <stdio.h>
#include <stdint.h>

int main(void) {
    int N;
    if (scanf("%d", &N) != 1) {
        return 1;
    }

    uint32_t result = 0;
    for (int i = 0; i < N; i++) {
        uint32_t val;
        scanf("%u", &val);
        result ^= val;
    }

    printf("%u\n", result);
    return 0;
}
