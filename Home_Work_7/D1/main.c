void btUpView(tree *root) {
    if (root == NULL) return;

    // Элемент очереди: узел + горизонтальная координата
    typedef struct {
        tree *node;
        int col;
    } QItem;

    #define MAXCOL 2001
    #define OFFSET  1000
    #define QCAP  20000   // ёмкость очереди с запасом

    // Очередь на динамическом массиве (кольцевой буфер)
    QItem *qdata = (QItem*)malloc(QCAP * sizeof(QItem));
    int front = 0, rear = -1, size = 0;

    // Храним первый узел для каждой колонки
    tree *top[MAXCOL] = {NULL};
    int minCol = 2147483647, maxCol = -2147483648;  // INT_MAX / INT_MIN

    // Кладём корень
    rear = (rear + 1) % QCAP;
    qdata[rear].node = root;
    qdata[rear].col = 0;
    size++;

    while (size > 0) {
        QItem cur = qdata[front];
        front = (front + 1) % QCAP;
        size--;

        int col = cur.col;
        tree *node = cur.node;

        int idx = col + OFFSET;
        if (idx >= 0 && idx < MAXCOL && top[idx] == NULL) {
            top[idx] = node;
            if (col < minCol) minCol = col;
            if (col > maxCol) maxCol = col;
        }

        if (node->left != NULL) {
            rear = (rear + 1) % QCAP;
            qdata[rear].node = node->left;
            qdata[rear].col = col - 1;
            size++;
        }
        if (node->right != NULL) {
            rear = (rear + 1) % QCAP;
            qdata[rear].node = node->right;
            qdata[rear].col = col + 1;
            size++;
        }
    }

    for (int c = minCol; c <= maxCol; c++) {
        int idx = c + OFFSET;
        if (top[idx] != NULL)
            printf("%d ", top[idx]->key);
    }

    free(qdata);
}