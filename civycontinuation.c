#include "civycontinuation.h"

struct _cvcontext {
    int state;
    void *covars;
};


struct _cvcontinuation {
    struct _cvcontext context;
    CVCallbackFunc *cocall;
    CVCleanupFunc *coclean;
    PyObject *coargs[3];
};


static void cv_dealloc_continuation(CVContinuation c)
{
    CVCleanupFunc cleanup = c->coclean;
    cv_dealloc_args(c->coargs);
    cleanup(((CVContext)c)->vars);
}


static void cv_dealloc_args(PyObject *args)
{
    Py_CLEAR(args[0]);
    Py_CLEAR(args[1]);
    Py_CLEAR(args[2]);
    
}
