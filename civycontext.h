#ifndef civycontext_included
#define civycontext_included

#include "q.h"
#include <Python.h>
#include "greenlet.h"

PyObject *CVAbdicate;
PyObject *CVYield;

typedef _QEntry _CVContext {
    PyObject *handler;
    PyGreenlet *loop;
    CVContext *parent;
    Q *cvthreads;
    } CVContext;


typedef _QEntry whatever {
    PyGreenlet *cvthread;
    };


void CVContext_dealloc(CVContext *self);
void CVThreads_push(CVContext *self, PyGreenlet *new_entry);
PyGreenlet* CVThreads_pop(CVContext *self);

#endif
