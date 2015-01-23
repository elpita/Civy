/* timer struct ************************************************************************************************************ */
struct _cvtimerstruct {
    PyObject_HEAD
    PyObject *func;
    PyObject *weak_actor;
    //PyObject *frame;
    //int recursion_depth;
    SDL_TimerID timer_id;
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

Uint32 inline_do_schedule(Uint32 interval, void *param)
{
    PyObject *actor, *args;
    struct _cvtimerstruct *pyparams;

    pyparams = (struct _cvtimerstruct *)param;
    actor = pyparams->weak_actor;

    if (is_dead(actor)) {
        return 0;
    }
    Py_INCREF(actor);
    args = Py_BuildValue("(OI)", actor, interval);
    
    if (args == NULL) {
        return 0;
    }
    else if (async_dispatch(actor, pyparams->func, args, NULL) < 0) {
        Py_DECREF(args);
        return 0;
    }
    return interval;
}


Uint32 cv_schedule_interval(Uint32 interval, void *param)
{
    CV_BEGIN_DENY_THREADS

    if (!inline_do_schedule(interval, param)) {
        fail_thread();
        return 0;
    }
    
    CV_END_DENY_THREADS

    return interval;


Uint32 cv_schedule_once(Uint32 interval, void *param)
{
    PyObject *actor, *dict;

    CV_BEGIN_DENY_THREADS

    if (!inline_do_schedule(interval, param)) {
        fail_thread();
        return 0;
    }
    /* Remove param from actor's dict */

    CV_END_DENY_THREADS

    return 0;
}


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
            PyObject *weak_func;
            {
                PyObject *func;
                PyObject *meth = PyObject_GetAttr((PyObject *)self, name);

                if (meth == NULL) {
                    return NULL;
                }
                func = PyMethod_GET_FUNCTION(meth);
                Py_DECREF(meth);
                weak_func = PyWeakref_NewProxy(func, NULL);
            }
            if (weak_func == NULL) {
                return NULL;
            }
            else {
                PyObject *weak_self = PyWeakref_NewRef((PyObject *)self, NULL);

                if (weak_self == NULL) {
                    Py_DECREF(weak_func);
                    return NULL;
                }
                PyTuple_SET_ITEM(args, 0, weak_self);

                if (schedule_rtp(coro, weak_func, args, kwds) < 0) {
                    Py_DECREF(weak_self);
                    Py_DECREF(weak_func);
                    return NULL;
                }
                longjmp(back, 1);
            }
        }
    }
}


static PyObject* EventDispatcher_schedule_interval(CVEventDispatcher self, PyObject *args, PyObject *kwds)
{
    Uint32 delay;
    SDL_TimerID my_timer_id;
    PyObject *callable, *timer_ids=self->timer_ids;
    static char *kwargs[] = {"callback", "interval", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OI", kwargs, &callable, &interval)) {
        return -1;
    }
    else if (!PyCallable_Check(callable)) {
        PyErr_Format(PyExc_TypeError, "%s is not callable.", ???); //fix this
        return NULL;
    }
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
