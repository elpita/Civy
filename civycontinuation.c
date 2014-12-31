#include "civycontinuation.h"

struct _cvcontext {
    int state;
    PyObject *passaround;
    void *vars;
};


struct _cvcontinuation {
    struct _cvcontext context;
    PyObject *argsptr[3];
};


static void cv_dealloc_args(PyObject *args)
{
    Py_XDECREF(args[0]);
    Py_XDECREF(args[1]);
    Py_XDECREF(args[2]);
}
