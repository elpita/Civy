#ifndef civycontext_included
#define civycontext_included

#include "q.h"
#include "greenlet.h"


typedef struct {
    PyObject_HEAD
} cv_ForkSentinel;
static PyTypeObject cv_ForkSentinelType;


typedef cv_ForkSentinel {
    PyObject *data;
} cv_WaitSentinel;
static PyTypeObject cv_WaitSentinelType;


typedef _QEntry _CVContext {
    PyObject *handler;
    PyGreenlet *spawn;
    CVContext *parent;
    Q *pipeline;
} CVContext;


typedef _QEntry whatever {
    PyGreenlet *cvthread;
};


int CVContext_dealloc(CVContext *self);
void CVThreads_push(CVContext *self, PyGreenlet *new_entry);
PyGreenlet* CVThreads_pop(CVContext *self);


#define Fork_Check(op)    ((op <> NULL) && PyObject_TypeCheck(op, &cv_ForkSentinelType))
#define Wait_Check(op)    ((op <> NULL) && PyObject_TypeCheck(op, &cv_WaitSentinelType))
#endif
