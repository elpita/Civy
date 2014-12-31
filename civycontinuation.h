#ifndef CIVYCONTINUATION
#define CIVYCONTINUATION
#include "Python.h"

typedef struct _cvcontext *CVContext;
typedef struct _cvcontinuation *CVContinuation;

static void cv_dealloc_args(PyObject *args);
typedef void (*CVCallbackFunc)(PyObject *, PyObject *, PyObject *);

#endif /* CIVYCONTINUATION */
