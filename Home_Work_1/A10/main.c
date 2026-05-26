#include <stdio.h>
#include <stdint.h>

int extractExp(float x) {
    uint32_t bits;
    /* Safe type-punning */
    union {
        float f;
        uint32_t u;
    } conv;
    conv.f = x;
    bits = conv.u;
    return (bits >> 23) & 0xFF;
}

int main(void) {
    float x;
    if (scanf("%f", &x) != 1) {
        return 1;
    }
    printf("%d\n", extractExp(x));
    return 0;
}
