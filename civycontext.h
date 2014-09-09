#include "q.h"

typedef _QEntry _CVContext {
    PyObject *master;
    CVContext *parent_chain;
    Q *cvthreads;
    } CVContext;


typedef _QEntry whatever {
    PyGreenlet *cvthread;
    };


void CVContext_dealloc(CVContext *self);
void CVThreads_push(CVContext *self, PyGreenlet *new_entry);
PyGreenlet* CVThreads_pop(CVContext *self);
