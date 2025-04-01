#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node {
    int data;
    int priority;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
} PriorityList;

typedef struct {
    PriorityList priorities[256];
} PriorityQueue;

void init_queue(PriorityQueue* q) {
    for (int i = 0; i < 256; i++) {
        q->priorities[i].head = NULL;
        q->priorities[i].tail = NULL;
    }
}

int enqueue(PriorityQueue* q, int data, int priority) {
    if (priority < 0 || priority > 255) return -1;
    
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) return -1;
    
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;

    if (q->priorities[priority].tail) {
        q->priorities[priority].tail->next = newNode;
    } else {
        q->priorities[priority].head = newNode;
    }
    q->priorities[priority].tail = newNode;
    
    return 0;
}

int dequeue_front(PriorityQueue* q, int* data) {
    for (int p = 0; p < 256; p++) {
        if (q->priorities[p].head) {
            Node* node = q->priorities[p].head;
            *data = node->data;
            
            q->priorities[p].head = node->next;
            if (!q->priorities[p].head) {
                q->priorities[p].tail = NULL;
            }
            
            free(node);
            return 0;
        }
    }
    return -1;
}

int dequeue_priority(PriorityQueue* q, int p, int* data) {
    if (p < 0 || p > 255) return -1;
    
    if (q->priorities[p].head) {
        Node* node = q->priorities[p].head;
        *data = node->data;
        
        q->priorities[p].head = node->next;
        if (!q->priorities[p].head) {
            q->priorities[p].tail = NULL;
        }
        
        free(node);
        return 0;
    }
    return -1;
}

int dequeue_min_priority(PriorityQueue* q, int min_p, int* data) {
    if (min_p < 0) min_p = 0;
    if (min_p > 255) min_p = 255;
    
    for (int p = 0; p <= min_p; p++) {
        if (q->priorities[p].head) {
            Node* node = q->priorities[p].head;
            *data = node->data;
            
            q->priorities[p].head = node->next;
            if (!q->priorities[p].head) {
                q->priorities[p].tail = NULL;
            }
            
            free(node);
            return 0;
        }
    }
    return -1;
}

void print_queue(PriorityQueue* q) {
    printf("\nТекущее состояние очереди:\n");
    for (int p = 0; p < 256; p++) {
        if (q->priorities[p].head) {
            printf("Приоритет %3d: [", p);
            Node* current = q->priorities[p].head;
            while (current) {
                printf(" %d", current->data);
                current = current->next;
            }
            printf(" ]\n");
        }
    }
}

void autogenerate(PriorityQueue* q, int count) {
    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        enqueue(q, rand() % 1000, rand() % 256);
    }
}

void print_menu() {
    printf("\n=== Меню управления очередью ===\n");
    printf("1. Добавить элемент\n");
    printf("2. Извлечь элемент с высшим приоритетом\n");
    printf("3. Извлечь элемент по конкретному приоритету\n");
    printf("4. Извлечь элемент с приоритетом не ниже чем заданный\n");
    printf("5. Показать очередь\n");
    printf("6. Автогенерация элементов\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}

int main() {
    PriorityQueue q;
    init_queue(&q);
    int choice, data, priority;

    do {
        print_menu();
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                printf("Данные: ");
                scanf("%d", &data);
                printf("Приоритет (0-255): ");
                scanf("%d", &priority);
                if (enqueue(&q, data, priority) == 0) {
                    printf("Добавлено!\n");
                } else {
                    printf("Ошибка!\n");
                }
                break;
                
            case 2:
                if (dequeue_front(&q, &data) == 0) {
                    printf("Извлечено: %d\n", data);
                } else {
                    printf("Очередь пуста!\n");
                }
                break;
                
            case 3:
                printf("Целевой приоритет: ");
                scanf("%d", &priority);
                if (dequeue_priority(&q, priority, &data) == 0) {
                    printf("Извлечено: %d (приоритет %d)\n", data, priority);
                } else {
                    printf("Элементы не найдены!\n");
                }
                break;
                
            case 4:
                printf("Макс. приоритет: ");
                scanf("%d", &priority);
                if (dequeue_min_priority(&q, priority, &data) == 0) {
                    printf("Извлечено: %d (приоритет <=%d)\n", data, priority);
                } else {
                    printf("Элементы не найдены!\n");
                }
                break;
                
            case 5:
                print_queue(&q);
                break;
                
            case 6:
                printf("Количество элементов: ");
                scanf("%d", &data);
                autogenerate(&q, data);
                printf("Сгенерировано %d элементов\n", data);
                break;
        }
    } while (choice != 0);

    // Очистка памяти
    for (int p = 0; p < 256; p++) {
        while (q.priorities[p].head) {
            Node* temp = q.priorities[p].head;
            q.priorities[p].head = temp->next;
            free(temp);
        }
    }

    return 0;
}