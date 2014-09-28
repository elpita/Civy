#ifndef civycontext_included
#define civycontext_included

#include "q.h"
#include "greenlet.h"


typedef struct _cvprocess *CVProcess;


static PyObject* CVProcess_loop(PyObject *capsule);
static CVProcess CVProcess_new(PyObject *event_handler);
static int CVProcess_dealloc(CVProcess self);
static int CVProcess_push_thread(CVProcess self, PyGreenlet *data);
static PyGreenlet* CVProcess_pop_thread(CVProcess self);
#endif
