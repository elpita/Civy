#ifndef CIVYOBJECTQUEUE_INCLUDED
#define CIVYOBJECTQUEUE_INCLUDED
#include "civycoroutine.h"

typedef struct _cvobjectqueue *CVObjectQ;

static void cv_init_object_queue(CVObjectQ self);
static int cv_object_queue_push(CVObjectQ self, CVCoroutine coro);
static CVCoroutine cv_object_queue_pop(CVObjectQ self);
static void cv_dealloc_object_queue(CVObjectQ self);

#endif /* CIVYOBJECTQUEUE_INCLUDED */
