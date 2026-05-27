#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_WORD_LEN 20

struct list {
    char word[MAX_WORD_LEN];
    struct list *next;
};

// Функция добавления слова в список
struct list* add_to_list(struct list *head, const char *word) {
    struct list *new_node = (struct list*)malloc(sizeof(struct list));
    if (new_node == NULL) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    strncpy(new_node->word, word, MAX_WORD_LEN - 1);
    new_node->word[MAX_WORD_LEN - 1] = '\0';
    new_node->next = head;
    return new_node;
}

// Функция обмена двух элементов списка
void swap_elements(struct list *a, struct list *b) {
    char temp[MAX_WORD_LEN];
    strcpy(temp, a->word);
    strcpy(a->word, b->word);
    strcpy(b->word, temp);
}

// Функция вывода списка
void print_list(struct list *head) {
    struct list *current = head;
    while (current != NULL) {
        printf("%s", current->word);
        if (current->next != NULL) {
            printf(" ");
        }
        current = current->next;
    }
    printf("\n");
}

// Функция удаления списка
void delete_list(struct list *head) {
    struct list *current = head;
    while (current != NULL) {
        struct list *temp = current;
        current = current->next;
        free(temp);
    }
}

// Функция сортировки списка (пузырьковая сортировка)
void sort_list(struct list *head) {
    int swapped;
    struct list *ptr1;
    struct list *lptr = NULL;
    
    if (head == NULL)
        return;
    
    do {
        swapped = 0;
        ptr1 = head;
        
        while (ptr1->next != lptr) {
            if (strcmp(ptr1->word, ptr1->next->word) > 0) {
                swap_elements(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

int main() {
    char input[1000];
    char *token;
    struct list *head = NULL;
    
    // Чтение строки ввода
    fgets(input, sizeof(input), stdin);
    
    // Удаление точки в конце строки
    input[strcspn(input, ".")] = '\0';
    
    // Разделение строки на слова
    token = strtok(input, " ");
    while (token != NULL) {
        head = add_to_list(head, token);
        token = strtok(NULL, " ");
    }
    
    // Сортировка списка
    sort_list(head);
    
    // Вывод результата
    print_list(head);
    
    // Удаление списка
    delete_list(head);
    
    return 0;
}