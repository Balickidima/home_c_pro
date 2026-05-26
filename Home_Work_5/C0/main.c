#include <stdio.h>
#include <string.h>

int main(void) {
    char s[1000];
    if (scanf("%999s", s) != 1) return 0;

    int n = strlen(s);
    int seen[1000] = {0};
    int count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            for (int k = j + 1; k < n; k++) {
                 /* трехзначное число не начинается с 0 */
                if (s[i] == '0') continue; 
                int num = (s[i] - '0') * 100 + (s[j] - '0') * 10 + (s[k] - '0');
                if (!seen[num]) {
                    seen[num] = 1;
                    count++;
                }
            }
        }
    }

    printf("%d\n", count);
    return 0;
}
