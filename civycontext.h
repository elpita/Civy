#include "q.h"

typedef _QEntry _CVContext {
    CVContext *old_chain;
    Q *cvthreads;
    } CVContext;


typedef _QEntry whatever {
    PyGreenlet *cvthread;
    };


int Q_is_empty(Q *q);
void Q_push(Q *self, QEntry *new_entry)
QEntry* Q_pop(Q *self);
