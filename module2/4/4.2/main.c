#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node {
    int data;
    int priority;
    struct Node* prev_global;
    struct Node* next_global;
    struct Node* prev_priority;
    struct Node* next_priority;
} Node;

typedef struct {
    Node* head_global;
    Node* tail_global;
    struct {
        Node* head;
        Node* tail;
    } priorities[256];
} PriorityQueue;

void init_queue(PriorityQueue* q) {
    q->head_global = NULL;
    q->tail_global = NULL;
    for (int i = 0; i < 256; i++) {
        q->priorities[i].head = NULL;
        q->priorities[i].tail = NULL;
    }
}

int enqueue(PriorityQueue* q, int data, int priority) {
    if (priority < 0 || priority > 255) {
        return -1;
    }
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        return -1;
    }
    newNode->data = data;
    newNode->priority = priority;
    newNode->prev_global = q->tail_global;
    newNode->next_global = NULL;
    newNode->prev_priority = NULL;
    newNode->next_priority = NULL;

    if (q->tail_global) {
        q->tail_global->next_global = newNode;
    } else {
        q->head_global = newNode;
    }
    q->tail_global = newNode;

    Node** p_head = &(q->priorities[priority].head);
    Node** p_tail = &(q->priorities[priority].tail);
    if (*p_tail) {
        (*p_tail)->next_priority = newNode;
        newNode->prev_priority = *p_tail;
        *p_tail = newNode;
    } else {
        *p_head = newNode;
        *p_tail = newNode;
    }

    return 0;
}

int dequeue_front(PriorityQueue* q, int* data) {
    if (!q->head_global) {
        return -1;
    }
    Node* node = q->head_global;
    *data = node->data;

    if (node->next_global) {
        node->next_global->prev_global = NULL;
        q->head_global = node->next_global;
    } else {
        q->head_global = NULL;
        q->tail_global = NULL;
    }

    int p = node->priority;
    Node** p_head = &(q->priorities[p].head);
    Node** p_tail = &(q->priorities[p].tail);

    if (node->prev_priority) {
        node->prev_priority->next_priority = node->next_priority;
    } else {
        *p_head = node->next_priority;
    }
    if (node->next_priority) {
        node->next_priority->prev_priority = node->prev_priority;
    } else {
        *p_tail = node->prev_priority;
    }

    free(node);
    return 0;
}

int dequeue_priority(PriorityQueue* q, int p, int* data) {
    if (p < 0 || p > 255) {
        return -1;
    }
    Node* node = q->priorities[p].head;
    if (!node) {
        return -1;
    }
    *data = node->data;

    if (node->prev_global) {
        node->prev_global->next_global = node->next_global;
    } else {
        q->head_global = node->next_global;
    }
    if (node->next_global) {
        node->next_global->prev_global = node->prev_global;
    } else {
        q->tail_global = node->prev_global;
    }

    Node** p_head = &(q->priorities[p].head);
    Node** p_tail = &(q->priorities[p].tail);

    *p_head = node->next_priority;
    if (*p_head) {
        (*p_head)->prev_priority = NULL;
    } else {
        *p_tail = NULL;
    }

    free(node);
    return 0;
}

int dequeue_min_priority(PriorityQueue* q, int p, int* data) {
    if (p < 0) p = 0;
    if (p > 255) p = 255;
    for (int q_prio = p; q_prio < 256; q_prio++) {
        Node* node = q->priorities[q_prio].head;
        if (node) {
            *data = node->data;

            if (node->prev_global) {
                node->prev_global->next_global = node->next_global;
            } else {
                q->head_global = node->next_global;
            }
            if (node->next_global) {
                node->next_global->prev_global = node->prev_global;
            } else {
                q->tail_global = node->prev_global;
            }

            Node** pq_head = &(q->priorities[q_prio].head);
            Node** pq_tail = &(q->priorities[q_prio].tail);

            *pq_head = node->next_priority;
            if (*pq_head) {
                (*pq_head)->prev_priority = NULL;
            } else {
                *pq_tail = NULL;
            }

            free(node);
            return 0;
        }
    }
    return -1;
}

void print_menu() {
    printf("\n=== Меню управления очередью ===\n");
    printf("1. Добавить элемент\n");
    printf("2. Извлечь первый элемент\n");
    printf("3. Извлечь элемент по приоритету\n");
    printf("4. Извлечь элемент с приоритетом не ниже указанного\n");
    printf("5. Показать очередь\n");
    printf("6. Автогенерация элементов\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}

void print_queue(PriorityQueue* q) {
    printf("\nТекущее состояние очереди:\n");
    printf("Общий порядок: [");
    Node* current = q->head_global;
    while(current) {
        printf(" %d(%d)", current->data, current->priority);
        current = current->next_global;
    }
    printf(" ]\n");
    
    printf("По приоритетам:\n");
    for(int i = 0; i < 256; i++) {
        if(q->priorities[i].head) {
            printf("Приоритет %3d: [", i);
            Node* p = q->priorities[i].head;
            while(p) {
                printf(" %d", p->data);
                p = p->next_priority;
            }
            printf(" ]\n");
        }
    }
}

void autogenerate(PriorityQueue* q, int count) {
    srand(time(NULL));
    for(int i = 0; i < count; i++) {
        int data = rand() % 1000;
        int priority = rand() % 256;
        enqueue(q, data, priority);
    }
}

int main() {
    PriorityQueue q;
    init_queue(&q);
    int choice;
    int data, priority, result;

    do {
        print_menu();
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                printf("Введите данные (целое число): ");
                scanf("%d", &data);
                printf("Введите приоритет (0-255): ");
                scanf("%d", &priority);
                if(enqueue(&q, data, priority) == 0) {
                    printf("Элемент добавлен успешно!\n");
                } else {
                    printf("Ошибка добавления элемента!\n");
                }
                break;
                
            case 2:
                if(dequeue_front(&q, &data) == 0) {
                    printf("Извлечен элемент: %d\n", data);
                } else {
                    printf("Очередь пуста!\n");
                }
                break;
                
            case 3:
                printf("Введите целевой приоритет: ");
                scanf("%d", &priority);
                result = dequeue_priority(&q, priority, &data);
                if(result == 0) {
                    printf("Извлечен элемент %d с приоритетом %d\n", data, priority);
                } else if(result == -1) {
                    printf("Нет элементов с таким приоритетом!\n");
                }
                break;
                
            case 4:
                printf("Введите минимальный приоритет: ");
                scanf("%d", &priority);
                result = dequeue_min_priority(&q, priority, &data);
                if(result == 0) {
                    printf("Извлечен элемент %d с приоритетом >=%d\n", data, priority);
                } else {
                    printf("Нет подходящих элементов!\n");
                }
                break;
                
            case 5:
                print_queue(&q);
                break;
                
            case 6:
                printf("Сколько элементов сгенерировать? ");
                scanf("%d", &data);
                autogenerate(&q, data);
                printf("Сгенерировано %d случайных элементов\n", data);
                break;
                
            case 0:
                printf("Завершение работы...\n");
                break;
                
            default:
                printf("Неверный выбор!\n");
        }
        
    } while(choice != 0);

    while(dequeue_front(&q, &data) == 0);

    return 0;
}