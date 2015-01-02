#ifndef CIVYACTORQUEUE
#define CIVYACTORQUEUE
#include "civycoroutine.h"

typedef struct _cvactorqueue *CVActorQ;
typedef struct _cvactorqueueentry *CVActorQEntry;

static void Queue_push(CVActorQ self, CVActorQEntry new_entry);
static void Queue_prepend(CVActorQ self, CVActorQEntry new_entry);
static QEntry Queue_pop(CVActorQ self);
#define Q_IS_EMPTY(q) (q->head == NULL)
#define Queue_init(q) (q->head = q->tail = NULL);

#endif /* CIVYACTORQUEUE */
