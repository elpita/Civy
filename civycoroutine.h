#ifndef CIVYCOROUTINE
#define CIVYCOROUTINE
#include "civycoroutinestack.h"

typedef struct _cvcoroutine *CVCoroutine;

static CVCoroutine cv_create_coroutine(PyObject *actor);
static void cv_dealloc_coroutine(CVCoroutine self);

#endif /* CIVYCOROUTINE */
