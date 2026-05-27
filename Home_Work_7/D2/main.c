tree *findBrother(tree *root, int key) {
    if (root == NULL) return NULL;

    int capacity = 1024;
    tree **stack = (tree**)malloc(capacity * sizeof(tree*));
    if (stack == NULL) return NULL;

    int top = 0;
    stack[0] = root;

    while (top >= 0) {
        tree *node = stack[top--];

        if (node->key == key) {
            tree *parent = node->parent;
            tree *brother = NULL;
            if (parent != NULL) {
                brother = (parent->left == node) ? parent->right : parent->left;
            }
            free(stack);
            return brother;
        }

        // Добавляем правого потомка (чтобы левый обработался первым)
        if (node->right != NULL) {
            if (top + 1 >= capacity) {
                capacity *= 2;
                tree **new_stack = (tree**)realloc(stack, capacity * sizeof(tree*));
                if (new_stack == NULL) {
                    free(stack);
                    return NULL;
                }
                stack = new_stack;
            }
            stack[++top] = node->right;
        }

        // Добавляем левого потомка
        if (node->left != NULL) {
            if (top + 1 >= capacity) {
                capacity *= 2;
                tree **new_stack = (tree**)realloc(stack, capacity * sizeof(tree*));
                if (new_stack == NULL) {
                    free(stack);
                    return NULL;
                }
                stack = new_stack;
            }
            stack[++top] = node->left;
        }
    }

    free(stack);
    return NULL; // узел с ключом key не найден
}