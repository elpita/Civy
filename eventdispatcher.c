static int str_endswith(PyObject *key, const char *suffix)
{
    if (!PyString_Check(key)) {
        return 0;
    }
    else {
        Py_ssize_t len = PyString_GET_SIZE(key);
        
        if (len <= 6) {
            return 0;
        }
        else {
            const char *str = PyString_AS_STRING(key);
            return (strncmp(str + (len - 6), suffix, 6) == 0);
        }
    }
}


static CVCoroutine get_current_coroutine(PyObject *actor)
{
    CVCoroutine coro, parent=NULL;

    if (global_coroutine != NULL) {
        parent = *global_coroutine;

        if (PyObject_RichCompareBool(actor, (PyObject *)(parent->state->actor_ptr), Py_EQ)) {
            return parent;
        }
    }
    coro = cv_create_coroutine(actor);

    if (coro == NULL) {
        return NULL;
    }
    coro->state->parent = parent;
    return coro;
}


static PyObject* EventDispatcher_dispatch(CVEventDispatcher self, PyObject *args, PyObject *kwds)
{
    CVCoroutine coro = get_current_coroutine((PyObject *)self);

    if (coro == NULL) {
        return NULL;
    }
    else {
        PyObject *name = PyTuple_GET_ITEM(args, 0);
    
        if (!str_endswith(name, "_event")) {
            PyErr_SetString(PyExc_TypeError, "dispatch takes a string argument ending with '_event'"); //fix
            return NULL;
        }
        else if (!PyObject_HasAttr((PyObject *)self, name)) {
            PyErr_SetString(PyExc_TypeError, "No event found"); //fix
            return NULL;
        }
        else {
            PyObject *func, *weak_value, *weak_self;
            {
                PyObject *meth = PyObject_GetAttr((PyObject *)self, name);
        
                if (meth == NULL) {
                    return NULL;
                }
                func = PyMethod_GET_FUNCTION(meth);
            }
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
    
            if (schedule_rtp(coro, func, args, kwds) < 0) {
                Py_DECREF(weak_self);
                Py_DECREF(weak_value);
                return NULL;
            }
            longjmp(back, 1);
        }
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
