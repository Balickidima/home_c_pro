#include <stdio.h>

int main(void) {
    int N;
    if (scanf("%d", &N) != 1 || N <= 0) {
        return 1;
    }

    int max, count = 0;
    for (int i = 0; i < N; i++) {
        int val;
        scanf("%d", &val);
        if (i == 0) {
            max = val;
            count = 1;
        } else if (val > max) {
            max = val;
            count = 1;
        } else if (val == max) {
            count++;
        }
    }

    printf("%d\n", count);
    return 0;
}
