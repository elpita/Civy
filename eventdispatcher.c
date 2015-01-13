static int str_endswith(PyObject *key, const char *suffix)
{
    Py_ssize_t len;
    const char *str;

    if (!PyString_Check(key)) {
        return 0;
    }
    len = PyString_GET_SIZE(key);
    
    if (len <= 6) {
        return 0;
    }
    str = PyString_AS_STRING(key);
    return (strncmp(str + (len - 6), suffix, 6) == 0);


static void schedule(PyObject *a, PyObject *args, PyObject *kwds, CVCallbackFunc *func)
{
    static struct _cvcontinuation cfp = {{0, NULL}, cv_call_from_python, NULL, {NULL, NULL, NULL}};
    static struct _cvcontinuation rtp = {{0, NULL}, cv_return_to_python, NULL, {NULL, NULL, NULL}};

    if (coroutine == NULL) {
        fix_that();
    }
    else if (context != NULL) {
        cv_stack_push(*coroutine, *context);
    }
    rtp->argsptr[1] = (PyObject *)(PyThreadState_GET()->frame);
    cv_stack_push(*coroutine, &rtp);
    cfp->argsptr[0] = a;
    cfp->argsptr[1] = args;
    cfp->argsptr[2] = kwds;
    cv_stack_push(*coroutine, &cfp);
    sdl_schedule(PyTuple_GET_ITEM(args, 0));
}


static PyObject* EventDispatcher_dispatch(CVEventDispatcher self, PyObject *args, PyObject *kwds)
{
    PyObject *name;
    
    name = PyTuple_GET_ITEM(args, 0);
    if (!str_endswith(name, "_event")) {
        PyErr_SetString(PyExc_TypeError, "dispatch takes a string argument ending with '_event'"); //fix
        return NULL;
    }
    else if (!PyObject_HasAttr((PyObject *)self, name)) {
        PyErr_SetString(PyExc_TypeError, "No event found"); //fix
        return NULL;
    }
    else {
        PyObject *func, *weak_value, *weak_self, *ret;
        PyObject *meth = PyObject_GetAttr((PyObject *)self, name);

        if (meth == NULL) {
            return NULL;
        }
        func = PyMethod_GET_FUNCTION(meth);
        weak_value = PyWeakref_NewProxy(func, NULL);

        if (weak_value == NULL) {
            return NULL;
        }
        weak_self = PyWeakref_NewRef((PyObject *)self, NULL);

        if (weak_self == NULL) {
            Py_DECREF(weak_value);
            return NULL;
        }
        PyTuple_SET_ITEM(args, 0, weak_self);
        schedule(ret,...);
        return ret;
    }
}


/*
static PyObject* EventDispatcherType_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    char *name;
    Py_ssize_t pos = 0;
    PyObject *key, *value, *weak_value;

    while (PyDict_Next(kwds, &pos, &key, &value)) {
        if (str_endswith(key, "_event") && PyFunction_Check(value)) {
            weak_value = PyWeakref_NewProxy(value, NULL);

            if (weak_value == NULL) {
                return NULL;
            }
            else if (PyDict_SetItem(some_dict, key, weak_value) < 0) {
                Py_DECREF(weak_value);
                return NULL;
            }
            Py_DECREF(weak_value);
        }
    }
    return type->tp_base->tp_new(type, args, kwds);
*/
