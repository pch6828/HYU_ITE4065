#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_PRODUCER                32
#define NUM_CONSUMER                NUM_PRODUCER
#define NUM_THREADS                 (NUM_PRODUCER + NUM_CONSUMER)
#define NUM_ENQUEUE_PER_PRODUCER    1000000
#define NUM_DEQUEUE_PER_CONSUMER    NUM_ENQUEUE_PER_PRODUCER

bool flag_verification[NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER];
void enqueue(int key);
int dequeue();

// -------- Concurrent queue with lock-free list --------
struct QueueNode {
    int key;
    QueueNode* next;
};

QueueNode* head = NULL;
QueueNode* tail = NULL;

void init_queue(void) {
    // Initialize a sentinel node
    QueueNode* sentinel = (QueueNode*)malloc(sizeof(QueueNode));
    sentinel->next = NULL;
    head = sentinel;
    tail = sentinel;
}

void enqueue(int key) {
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    new_node->key = key;
    new_node->next = NULL;

#if 0
    // method 1 - less scalable, faster than method 2 if the number of thread is small
    while (__sync_bool_compare_and_swap(&tail->next, NULL, new_node) == false) {
        pthread_yield();
    }
    tail = new_node;
#else
    
    // method 2 - followed the method in the lecture slide. More scalable than method 1.
    QueueNode* tail_copied;
    while (1) {
        tail_copied = tail;
        QueueNode* ret_node = __sync_val_compare_and_swap(&tail_copied->next, NULL, new_node);
        if (ret_node == NULL) {
            // success to append
            break;
        } else {
            __sync_bool_compare_and_swap(&tail, tail_copied, ret_node);
        }
    }
    __sync_bool_compare_and_swap(&tail, tail_copied, new_node);
#endif
}

int dequeue(void) {
    QueueNode* sentinel_node;
    int ret_key;

    while (1) {
        sentinel_node = head;
        if (sentinel_node->next != NULL) {
            if (__sync_bool_compare_and_swap(&head, sentinel_node, sentinel_node->next)) {
                ret_key = sentinel_node->next->key;
                break;
            }
        } else {
            // Queue empty
            pthread_yield(); // blocking
        }
    }

    //free(sentinel_node);

    return ret_key;
}
// ------------------------------------------------

void* ProducerFunc(void* arg) {
    long tid = (long)arg;

    int key_enqueue = NUM_ENQUEUE_PER_PRODUCER * tid;
    for (int i = 0; i < NUM_ENQUEUE_PER_PRODUCER; i++) {
        enqueue(key_enqueue);
        key_enqueue++;
    }

    return NULL;
}

void* ConsumerFunc(void* arg) {
    for (int i = 0; i < NUM_DEQUEUE_PER_CONSUMER; i++) {
        int key_dequeue = dequeue();
        flag_verification[key_dequeue] = true;
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    init_queue();

    for (int i = 0; i < NUM_THREADS; i++) {
        if (i < NUM_PRODUCER) {
            pthread_create(&threads[i], 0, ProducerFunc, (void**)i);
        } else {
            pthread_create(&threads[i], 0, ConsumerFunc, NULL);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // verify
    for (int i = 0; i < NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER; i++) {
        if (flag_verification[i] == false) {
            printf("INCORRECT!\n");
            return 0;
        }
    }
    printf("CORRECT!\n");

    return 0;
}

