#include "civycontinuation.h"

struct _cvcontext {
    int state;
    void *vars;
};


struct _cvcontinuation {
    struct _cvcontext context;
    CVCallbackFunc *cocall;
    CVCleanupFunc *coclean;
    PyObject *argsptr[3];
};


static void cv_dealloc_continuation(CVContinuation c)
{
    CVCleanupFunc cleanup = c->coclean;
    cv_dealloc_args(c->argsptr);
    cleanup((CVContext)->vars);
}


static void cv_dealloc_args(PyObject *args)
{
    Py_XDECREF(args[0]);
    Py_XDECREF(args[1]);
    Py_XDECREF(args[2]);
    
}
