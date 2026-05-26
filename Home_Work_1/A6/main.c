#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t N;
    if (scanf("%u", &N) != 1) {
        return 1;
    }

    uint32_t result = N ^ 0xFF000000U;
    printf("%u\n", result);
    return 0;
}
