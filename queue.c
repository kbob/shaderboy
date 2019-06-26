#include "queue.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct queue {
    pthread_mutex_t lock;
    pthread_cond_t nonempty;
    pthread_cond_t nonfull;
    size_t size;
    size_t head;
    size_t tail;
};

static void queue_init(queue *q, size_t size)
{
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->nonempty, NULL);
    pthread_cond_init(&q->nonfull, NULL);
    q->size = size;
    q->head = 0;
    q->tail = 0;
}

static void queue_deinit(queue *q)
{
    pthread_mutex_destroy(&q->lock);
}

queue *create_queue(size_t size)
{
    queue *q = calloc(1, sizeof *q);
    queue_init(q, size);
    return q;
}

void destroy_queue(queue *q)
{
    queue_deinit(q);
    free(q);
}

static bool queue_is_empty(const queue *q)
{
    return q->head == q->tail;
}

static bool queue_is_full(const queue *q)
{
    return q->head != q->tail && (q->size + q->head - q->tail) % q->size == 0;
}

size_t queue_acquire_empty(queue *q)
{
    pthread_mutex_lock(&q->lock);
    while (queue_is_full(q)) {
        pthread_cond_wait(&q->nonfull, &q->lock);
    }
    size_t index = q->tail % q->size;
    pthread_mutex_unlock(&q->lock);
    return index;
}

void queue_release_full(queue *q)
{
    pthread_mutex_lock(&q->lock);
    q->tail = (q->tail + 1) % (2 * q->size);
    pthread_cond_signal(&q->nonempty);
    pthread_mutex_unlock(&q->lock);
}

size_t queue_acquire_full(queue *q)
{
    pthread_mutex_lock(&q->lock);
    while (queue_is_empty(q)) {
        pthread_cond_wait(&q->nonempty, &q->lock);
    }
    size_t index = q->head % q->size;
    pthread_mutex_unlock(&q->lock);
    return index;
}

void queue_release_empty(queue *q)
{
    pthread_mutex_lock(&q->lock);
    q->head = (q->head + 1) % (2 * q->size);
    pthread_cond_signal(&q->nonfull);
    pthread_mutex_unlock(&q->lock);
}
