#include "civycontinuation.h"


struct _cvcontext {
    int state;
    void *covars;
};


typedef struct _cvcontinuation {
    struct _cvcontext context;
    CVCallbackFunc cocall;
    CVCleanupFunc coclean;
    PyObject *coargs[3];
} CVContinuation;


static void cv_dealloc_args(PyObject **args)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    Py_CLEAR(args[0]);
    Py_CLEAR(args[1]);
    Py_CLEAR(args[2]);
    PyGILState_Release(gstate);
    
}


static void cv_dealloc_continuation(CVContinuation *c)
{
    CVCleanupFunc cleanup = c->coclean;

    cv_dealloc_args(c->coargs);

    if (cleanup) {
        cleanup(c->context->covars);
    }
}
