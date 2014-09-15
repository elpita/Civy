#ifndef civycontext_included
#define civycontext_included

#include "q.h"
#include "greenlet.h"


typedef QueueEntry _CVProcess *CVProcess;


int CVProcess_dealloc(CVProcess self);
void CVProcess_push_thread(CVProcess self, PyGreenlet *new_entry);
PyGreenlet* CVThreads_pop(CVProcess self);


typedef struct cv_ForkSentinel;
static PyTypeObject cv_ForkSentinelType;

typedef cv_ForkSentinel cv_WaitSentinel;
static PyTypeObject cv_WaitSentinelType;

#define Fork_Check(op)    PyObject_TypeCheck(op, &cv_ForkSentinelType)
#define Wait_Check(op)    PyObject_TypeCheck(op, &cv_WaitSentinelType)
#endif
