#include "q.h"

typedef _QEntry _CVContext {
    CVContext *old_chain;
    Q *cvthreads;
    } CVContext;


typedef _QEntry whatever {
    PyGreenlet *cvthread;
    };


void CVThreads_push(CVContext *self, PyGreenlet *new_entry);
PyGreenlet* CVThreads_pop(CVContext *self);
