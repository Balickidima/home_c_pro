#include <stdio.h>

int main(void) {
    long long N;
    if (scanf("%lld", &N) != 1 || N <= 0) {
        return 1;
    }

    int count = 0;
    long long pos_sum = 0;
    long long power = 1;

    while (N > 0) {
        int rem = N % 3;
        if (rem == 1) {
            count++;
            pos_sum += power;
            if (pos_sum > 1000000) {
                printf("-1\n");
                return 0;
            }
            N = N / 3;
        } else if (rem == 2) {
            count++;
            N = N / 3 + 1;
        } else {
            N = N / 3;
        }
        power *= 3;
    }

    printf("%d\n", count);
    return 0;
}
