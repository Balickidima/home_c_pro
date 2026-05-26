#include <stdint.h>
#include <stddef.h>

typedef struct list {
    uint64_t address;
    size_t size;
    char comment[64];
    struct list *next;
} list;

uint64_t findMaxBlock(list *head) {
    if (head == NULL) {
        return 0;
    }

    uint64_t max_address = head->address;
    size_t max_size = head->size;

    list *current = head->next;
    while (current != NULL) {
        if (current->size > max_size) {
            max_size = current->size;
            max_address = current->address;
        }
        current = current->next;
    }

    return max_address;
}
