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


/* timer threads *********************************************************************************************************** */
#define CV_BEGIN_DENY_THREADS { PyGILState_STATE gstate; gstate = PyGILState_Ensure();
#define CV_END_DENY_THREADS PyGILState_Release(gstate); }
#define fail_thread() do { Py_AddPendingCall(...); PyGILState_Release(gstate); } while(0)

/* EventDispatcher ********************************************************************************************************* */
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


static PyObject* EventDispatcher_dispatch(CVEventDispatcher self, PyObject *args, PyObject *kwds)
{
    CVCoroutine coro;
    PyObject *func, *positional_args;
    Py_ssize_t s_size = PyTuple_GET_SIZE(args);

    /* First, check if function call is legal */
    if (s_size < 1) {
        PyErr_SetString(PyExc_TypeError, "'dispatch' takes a string argument ending with '_event'"); //fix
        return NULL;
    }
    else {
        PyObject *name = PyTuple_GET_ITEM(args, 0);

        if (!str_endswith(name, "_event")) {
            PyErr_SetString(PyExc_TypeError, "'dispatch' takes a string argument ending with '_event'"); //fix
            return NULL;
        }
        else if (!PyObject_HasAttr((PyObject *)self, name)) {
            PyErr_SetString(PyExc_TypeError, "No event found"); //fix
            return NULL;
        }
        else {
            PyObject *meth = PyObject_GetAttr((PyObject *)self, name);

            if (meth == NULL) {
                return NULL;
            }
            func = PyMethod_GET_FUNCTION(meth);
            Py_INCREF(func);
            Py_DECREF(meth);
        }
    }

    /* Next, create the passaround positional arguments */
    {
        /* Make `self` argument a weak-reference to avoid unintentionally keeping it alive */
        PyObject *self_arg;
        PyObject *weak_self = PyWeakref_NewRef((PyObject *)self, NULL);

        if (weak_self == NULL) {
            Py_DECREF(func);
            return NULL;
        }
        self_arg = PyTuple_Pack(1, weak_self);

        if (self_arg == NULL) {
            Py_DECREF(weak_self);
            Py_DECREF(func);
            return NULL;
        }

        if (s_size > 1) {
            PyObject *tup = PyTuple_GetSlice(args, 1, s_size);

            if (tup == NULL) {
                Py_DECREF(self_arg);
                Py_DECREF(weak_self);
                Py_DECREF(func);
                return NULL;
            }
            positional_args = PySequence_Concat(self_arg, tup);
            Py_DECREF(tup);
            Py_DECREF(self_arg);
        }
        else {
            positional_args = self_arg;
        }
        Py_DECREF(weak_self);
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


static void cv_schedule_once(PyObject **self, PyObject **callback, Uint32 *delay, PyObject *ids)
{
    PyObject *key;
    long int lint;
    
    lint = PyObject_Hash(*callback);
    
    if (lint < 0) {
        return -1;
    }
    key = PyLong_FromLong(lint);
    
    if (key == NULL) {
        return -1;
    }
    else {
        PyObject *whatever = PyDict_GetItem(ids, key);

        if (whatever != NULL) {
            if (whatever->type == INTERVAL) {
                PyErr_Format(PyExc_RuntimeError, "%s is already a scheduled periodic function.", PYOBJECT_NAME(*callback));
                Py_DECREF(key);
                return -1;
            }
            my_timer_id = SDL_AddTimer(*delay, schedule_once, my_callback_param);

            if (!my_timer_id) {
                PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
                Py_DECREF(key);
                return -1;
            }
            new_id = Py_BuildValue("i", my_timer_id);

            if ((new_id == NULL) || (PyList_Append(whatever->sdl_ids, new_id) < 0)) {
                Py_DECREF(key);
                return -1;
            }
            Py_INCREF(whatever);
            Py_DECREF(new_id);
        }
    }
}





static whatever
    my_timer_id = SDL_AddTimer(delay, my_callbackfunc, my_callback_param);

    if (!my_timer_id) {
        PyErr_SetString(PyExc_TypeError, SDL_GetError());
        return NULL;
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
