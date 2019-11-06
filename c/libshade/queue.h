#ifndef QUEUE_included
#define QUEUE_included

#include <stddef.h>

typedef struct queue queue;

extern queue *create_queue(size_t size);
extern void   destroy_queue(queue *q);

extern size_t queue_acquire_empty(queue *q);
extern void   queue_release_full(queue *q);
extern size_t queue_acquire_full(queue *q);
extern void   queue_release_empty(queue *q);

#endif /* !QUEUE_included */
