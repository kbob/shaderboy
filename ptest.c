#include <pthread.h>
#include <stdio.h>

#include "queue.h"

typedef struct queues {
    queue *a;
    queue *b;
    char b0[10];
    char b1[10];
} queues;

static void *produce(void *user_data)
{
    queues *qs = user_data;
    for (const char *src = "_Hello, World!\n"; *src; src++) {
        size_t i = queue_acquire_empty(qs->a);
        qs->b0[i] = src[1];
        queue_release_full(qs->a);
    }
    return NULL;
}

static void *mediate(void *user_data)
{
    queues *qs = user_data;
    while (1) {
        size_t i0 = queue_acquire_full(qs->a);
        char c = qs->b0[i0];
        queue_release_empty(qs->a);
        size_t i1 = queue_acquire_empty(qs->b);
        qs->b1[i1] = c;
        queue_release_full(qs->b);
        if (c == 0)
            break;
    }
    return NULL;
}

static void *consume(void *user_data)
{
    queues *qs = user_data;
    while (1) {
        size_t index = queue_acquire_full(qs->b);
        char c = qs->b1[index];
        queue_release_empty(qs->b);
        if (c == '\0')
            break;
        putchar(c);
        fflush(stdout);
    }
    return NULL;
}

int main(void)
{
    queues qs;
    qs.a = create_queue(4);
    qs.b = create_queue(3);

    pthread_t producer, middle, consumer;

    pthread_create(&producer, NULL, produce, &qs);
    pthread_create(&middle, NULL, mediate, &qs);
    pthread_create(&consumer, NULL, consume, &qs);
    pthread_join(producer, NULL);
    pthread_join(middle, NULL);
    pthread_join(consumer, NULL);
    return 0;
}
