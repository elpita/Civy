#ifndef CIVYCOROUTINESTACK
#define CIVYCOROUTINESTACK
#include "civycontinuation.h"

typedef struct _cvcostack *CVCoStack;

static CVCoStack cv_init_costack(CVCoStack s);
static void cv_dealloc_costack(CVCoStack s);
static int cv_costack_push(CVCoStack s, CVContinuation c);
static CVContinuation cv_costack_pop(CVCoStack s);

#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 128
#endif /* CV_STACK_LENGTH */

#endif /* CIVYCOROUTINESTACK */
