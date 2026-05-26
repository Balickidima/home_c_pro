#include <stdio.h>

int main(void) {
    int N;
    if (scanf("%d", &N) != 1) {
        return 1;
    }

    int ch;
    while ((ch = getchar()) != EOF) {
        if (ch == '.') {
            putchar('.');
            break;
        } else if (ch >= 'a' && ch <= 'z') {
            putchar('a' + (ch - 'a' + N) % 26);
        } else if (ch >= 'A' && ch <= 'Z') {
            putchar('A' + (ch - 'A' + N) % 26);
        } else {
            putchar(ch);
        }
    }

    return 0;
}
