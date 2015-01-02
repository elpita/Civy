#ifndef CIVYACTORQUEUE
#define CIVYACTORQUEUE
#include "civycoroutine.h"

typedef struct _cvactorqueue *CVActorQ;
typedef struct _cvactorqueueentry *CVActorQEntry;

static int cv_actor_queue_push(CVActorQ self, CVCoroutine coro);
static CVCoroutine cv_actor_queue_pop(CVActorQ self);
#define Q_IS_EMPTY(q) (q->head == NULL)
#define Queue_init(q) (q->head = q->tail = NULL)

#endif /* CIVYACTORQUEUE */
