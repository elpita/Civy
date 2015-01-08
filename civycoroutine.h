#ifndef CIVYCOROUTINE
#define CIVYCOROUTINE
#include "civycoroutinestack.h"

typedef struct _cvcoroutine *CVCoroutine;

static CVCoroutine cv_create_coroutine(PyObject *actor);
static void cv_dealloc_coroutine(CVCoroutine self);

#define is_dead(a) ((a == Py_None) || _is_dead(a))

#endif /* CIVYCOROUTINE */
