#define CV_BEGIN_DENY_THREADS {PyGILState_STATE gstate; gstate = PyGILState_Ensure();
#define CV_COLLAPSE_THREAD() Py_AddPendingCall(&_cv_fail, NULL); PyGILState_Release(gstate); return 0
#define CV_END_DENY_THREADS PyGILState_Release(gstate); }


static int _cv_fail(void *)
{
    cv_longjmp(to_cv_end, -1);
}


static Uint32 cv_periodic_function(Uint32 interval, void *param)
{
    PyObject *v;
    CVCoroutine *coro;
    Something *this = (Something *)param;
    
    CV_BEGIN_DENY_THREADS
    
    /* Pass the interval's value to the coroutine */
    v = Py_BuildValue("(OI)", this->weak_actor, interval);

    if (v == NULL) {
        CV_COLLAPSE_THREAD();
    }
    else if ((cv_spawn(this->coro, this->func, v, NULL) < 0) || (cv_push_event(this->weak_actor, CV_DISPATCHED_EVENT, 0) < 0) || (cv_object_queue_push(this->q, this->coro) < 0)) {
        Py_DECREF(v);
        CV_COLLAPSE_THREAD();
    }
    Py_INCREF(this->func);

    /* Create a new coroutine for the next period */
    coro = cv_create_coroutine(this->actor);

    if (coro == NULL) {
        CV_COLLAPSE_THREAD();
    }
    else if (cv_schedule_period(this->actor, this->ids, this->key) < 0) {
        CV_COLLAPSE_THREAD();
    }
    Py_INCREF(ids);
    Py_INCREF(key);
    this->coro = coro;

    CV_END_DENY_THREADS

    return interval;
}


static int cv_schedule_period(CVCoroutine *coro, PyObject *ids, PyObject *key)
{
    _cvcontinuation period = {{0, NULL}, cv_periodic_exec, NULL, {NULL, NULL, NULL}};
    PyObject *arguments[3] = {ids, key, NULL};

    period.coargs = arguments;

    if (cv_costack_push(coro, &period) < 0) {
        return -1;
    }
    return 0;
}


static int cv_schedule_interval(PyObject *self, const char *name, Uint32 delay, PyObject *ids)
{
    PyObject *func;
    Something *t_struct;
    PyObject *weak_actor;
    PyObject *key;

    if (PyDict_GetItemString(ids, name) != NULL) {
        PyErr_Format(PyExc_RuntimeError, "'%s' is already a scheduled periodic event.", name);
        return -1;
    }
    else {
        PyObject *meth = PyObject_GetAttrString(self, name);

        if (meth == NULL) {
            PyErr_Format(PyExc_AttributeError, "'%s' has no event named '%s'.", PYOBJECT_NAME(self), name);
            return -1;
        }
        else if (!PyMethod_Check(meth)) {
            PyErr_Format(PyExc_AttributeError, "'%s' has no event named '%s'.", PYOBJECT_NAME(self), name);
            Py_DECREF(meth);
            return -1;
        }
        func = PyMethod_Function(meth);
        Py_INCREF(func);
        Py_DECREF(meth);
    }
    weak_actor = PyWeakref_NewRef(self, NULL);
    
    if (weak_actor == NULL) {
        Py_DECREF(func);
        return -1;
    }
    key = PyString_FromString(name);
    
    if (key == NULL) {
        Py_DECREF(weak_actor);
        Py_DECREF(func);
        return -1;
    }
    coro = cv_create_coroutine(self);
    
    if (coro == NULL) {
        Py_DECREF(key);
        Py_DECREF(weak_actor);
        Py_DECREF(func);
        return -1;
    }
    else if (cv_schedule_period(coro, ids, key) < 0) {
        cv_dealloc_coroutine(coro);
        Py_DECREF(key);
        Py_DECREF(weak_actor);
        Py_DECREF(func);
        return -1;
    }
    Py_INCREF(ids);
    Py_INCREF(key);
    t_struct = new_tstruct();

    if (t_struct == NULL) {
        cv_dealloc_coroutine(coro);
        Py_DECREF(key);
        Py_DECREF(weak_actor);
        Py_DECREF(func);
        return -1;
    }
    t_struct->actor = self;
    t_struct->q = &(((CVObject *)self)->cvprocesses);
    t_struct->coro = coro;
    t_struct->weak_actor = weak_actor;
    t_struct->key = key;
    t_struct->func = func;
    t_struct->timer_id = SDL_AddTimer(delay, &cv_periodic_function, (void *)t_struct);

    if (!timer_id) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        destroy(t_struct); //Will take care of the rest.
        return -1;
    }
    else if (PyDict_SetItemString(ids, name, (PyObject *)t_struct) < 0) {
        destroy(t_struct);
        return -1;
    }
    return 0;
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


static PyObject* EventDispatcher_schedule(CVEventDispatcher self, PyObject *args, PyObject *kwargs)
{
    int repeat=0;
    Uint32 delay;
    char *name;

    {
        static char *kwds[] = {"name", "delay", "repeat", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sI|i", kwds, &name, &delay, &repeat)) {
            return NULL;
        }
        else if (!str_startswith(name, "on_")) {
            PyErr_SetString(PyExc_ValueError, "'schedule' takes a string argument starting with 'on_'.");
            return NULL;
        }
        else if (delay <= 0) {
            PyErr_SetString(PyExc_ValueError, "'delay' must be greater than 0 miliseconds.");
            return NULL;
        }
    }

    if (!repeat && (cv_schedule_once(self, name, delay, self->timer_ids) < 0)) {
        return NULL;
    }
    else if (cv_schedule_interval(self, name, delay, self->time_ids) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}
