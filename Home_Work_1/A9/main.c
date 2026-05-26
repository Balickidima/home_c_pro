#include <stdio.h>
#include <stdint.h>

struct pack_array {
    uint32_t array;
    uint32_t count0 : 8;
    uint32_t count1 : 8;
};

void array2struct(int arr[], struct pack_array *p) {
    p->array = 0;
    p->count0 = 0;
    p->count1 = 0;
    for (int i = 0; i < 32; i++) {
        p->array |= ((uint32_t)arr[i] << (31 - i));
        if (arr[i] == 0)
            p->count0++;
        else
            p->count1++;
    }
}

int main(void) {
    int arr[32];
    for (int i = 0; i < 32; i++) {
        scanf("%d", &arr[i]);
    }

    struct pack_array p;
    array2struct(arr, &p);
    printf("%u %u %u\n", p.array, p.count0, p.count1);
    return 0;
}
