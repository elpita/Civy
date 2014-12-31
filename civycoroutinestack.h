#ifndef CIVYCOROUTINESTACK
#define CIVYCOROUTINESTACK
#include "civycontinuation.h"

typedef struct _cvstack *CVStack;

static CVStack cv_init_stack(CVStack s);
static void cv_dealloc_stack(CVStack s);
static void cv_stack_push(CVStack s, CVContinuation val);
static CVContinuation cv_stack_pop(CVStack s);

#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 64
#endif /* CV_STACK_LENGTH */

#endif /* CIVYCOROUTINESTACK */
