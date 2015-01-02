#ifndef CIVYOBJECTQUEUE
#define CIVYOBJECTQUEUE
#include "civycoroutine.h"

typedef struct _cvactorqueue *CVObjectQ;

static int cv_actor_queue_push(CVObjectQ self, CVCoroutine coro);
static CVCoroutine cv_actor_queue_pop(CVObjectQ self);
#define Queue_init(q) (q->head = q->tail = NULL)

#endif /* CIVYOBJECTQUEUE */
