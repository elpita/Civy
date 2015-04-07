/* timer struct ************************************************************************************************************ */

struct _cvtimerstruct {
    PyObject_HEAD
    enum {ONCE, INTERVAL} type;
    PyObject *func;
    PyObject *weak_actor;
    PyObject *sdl_ids;
}


static void cvtimerstruct_dealloc(struct _cvtimerstruct *self)
{
    Py_DECREF(self->weak_actor);
    PyObject_Del( (PyObject *)self );
}


static PyTypeObject CVTimerStructType = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "timerstruct",                      /* tp_name */
    sizeof(struct _cvtimerstruct),      /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)cvtimerstruct_dealloc,  /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_compare */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    0,                                  /* tp_flags */
    "CV timer data",                    /* tp_doc */
};


/* EventDispatcher ********************************************************************************************************* */
static int str_startswith(const char *key, const char *prefix)
{
    size_t lenpre = strlen(prefix),
           lenstr = strlen(key);
    return lenstr < lenpre ? 0 : (strncmp(prefix, key, lenpre) == 0);
}


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


static PyObject* EventDispatcher_dispatch(CVEventDispatcher *self, PyObject *args, PyObject *kwds)
{
    CVCoroutine *coro;
    PyObject *actor_ptr, *name, *p_args, *frame;
    Py_ssize_t s_size = PyTuple_GET_SIZE(args);

    /* First, check if function call is legal */
    if (PyGILState_GetThisThreadState()) {
        /* CV FAIL THREAD */
    }
    else if (s_size < 1) {
        PyErr_SetString(PyExc_ValueError, "'dispatch' takes a string argument ending with '_event'"); //fix
        return NULL;
    }
    name = PyTuple_GET_ITEM(args, 0);

    if (!str_endswith(name, "_event")) {
        PyErr_SetString(PyExc_TypeError, "'dispatch' takes a string argument ending with '_event'"); //fix
        return NULL;
    }
    else if (!PyObject_HasAttr((PyObject *)self, name)) {
        PyErr_SetString(PyExc_TypeError, "No event found"); //fix
        return NULL;
    }
    else {
        actor_ptr = ((CVObject *)self)->proxy_ref;

        if (s_size > 1) {
            p_args = PyTuple_GetSlice(args, 1, s_size);
        }
        else {
            p_args = PyTuple_New(0);
        }

        if (p_args == NULL) {
            return NULL;
        }
    }

    if (!PyObject_RichCompareBool((PyObject *)self, global_actor, Py_EQ)) {
        frame = (PyObject *)PyEval_GetFrame();
        _main_thread = PyEval_SaveThread();

        coro = cv_create_coroutine(actor_ptr, _cv_current_coroutine);

        if (coro == NULL) {
            PyEval_RestoreThread(_main_thread);
            Py_DECREF(p_args);
            return NULL;
        }
        else if (!cv_costack_push(coro->stack, cv_create_continuation(coro, frame, NULL, NULL, cv_join, NULL, NULL))) {
            cv_dealloc_coroutine(coro);
            PyEval_RestoreThread(_main_thread);
            Py_DECREF(p_args);
            return NULL;
        }
        else if (!cv_costack_push(coro->stack, cv_create_continuation(coro, actor_ptr, name, p_args, cv_exec, NULL, NULL))) {
            cv_dealloc_coroutine(coro);
            PyEval_RestoreThread(_main_thread);
            Py_DECREF(p_args);
            return NULL;
        }
    }
    else {
        _main_thread = PyEval_SaveThread();
        coro = _cv_current_coroutine;

        if (!cv_costack_push(coro->stack, cv_create_continuation(coro, frame, NULL, NULL, cv_join, NULL, NULL))) {
            PyEval_RestoreThread(_main_thread);
            Py_DECREF(p_args);
            return NULL;
        }
        else if (!cv_costack_push(coro->stack, cv_create_continuation(coro, actor_ptr, name, p_args, cv_exec, NULL, NULL))) {
            PyEval_RestoreThread(_main_thread);
            Py_DECREF(p_args);
            return NULL;
    }
    /* Finally, create the coroutine and fork the 'thread' */
    coro = get_current_coroutine((PyObject *)self);

    if (coro == NULL) {
        Py_DECREF(positional_args);
        Py_DECREF(func);
        return NULL;
    }
    else if (cv_fork(coro, func, positional_args, kwds) < 0) {
        Py_DECREF(positional_args);
        Py_DECREF(func);
        return NULL;
    }
    cv_longjmp(back, 1);
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
