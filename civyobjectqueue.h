#ifndef CIVYOBJECTQUEUE_INCLUDED
#define CIVYOBJECTQUEUE_INCLUDED
#include "civycoroutine.h"

typedef struct _cvobjectqueue *CVObjectQ;

static int cv_object_queue_push(CVObjectQ self, CVCoroutine coro);
static CVCoroutine cv_object_queue_pop(CVObjectQ self);
#define Queue_init(q) (q->head = q->tail = NULL)

#endif /* CIVYOBJECTQUEUE_INCLUDED */
