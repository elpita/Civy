typedef void (*CVCallbackFunc)(PyObject *, PyObject *, PyObject *);
typedef void (*CVCleanupFunc)(void *);


typedef struct _cvcontinuation {
    int state;
    void *covars;
    CVCallbackFunc cocall;
    CVCleanupFunc coclean;
    PyObject *coargs[3];
} CVContinuation;


static CVContinuation cv_create_continuation(PyObject *actor_ptr, PyObject *args, PyObject *kwargs, CVCallbackFunc cocall, CVCleanupFunc coclean, void *covars)
{
    CVContinuation self = {{0, covars}, cocall, coclean, {actor_ptr, args, kwargs}};
    {
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_INCREF(actor_ptr);
        Py_XINCREF(args);
        Py_XINCREF(kwargs);
        PyGILState_Release(gstate);
    }
    return self;
}


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
        void *vars = c->context->covars;
        cleanup(vars);
    }
}
