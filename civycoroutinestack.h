#ifndef CIVYCOROUTINESTACK
#define CIVYCOROUTINESTACK
#include "civycontinuation.h"

typedef struct _CVCoStack *CVCoStack;

static CVCoStack cv_init_stack(CVCoStack s);
static void cv_dealloc_stack(CVCoStack s);
static void cv_stack_push(CVCoStack s, CVContinuation val);
static CVContinuation cv_stack_pop(CVCoStack s);

#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 64
#endif /* CV_STACK_LENGTH */

#endif /* CIVYCOROUTINESTACK */
