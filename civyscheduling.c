#define CV_BEGIN_DENY_THREADS {PyGILState_STATE gstate; gstate = PyGILState_Ensure();
#define CV_COLLAPSE_THREAD() PyGILState_Release(gstate); Py_AddPendingCall(&_cv_fail, NULL); return 0
#define CV_END_DENY_THREADS PyGILState_Release(gstate); }
#define slate_doc_string ""


/* Periodic Event Dispatching ********************************************************************************************* */
typedef struct _cvperiodicslate {
    PyObject_HEAD
    PyObject *weak_actor;
    PyObject *actor;
    CVObjectQ *q;
    CVCoroutine *coro;
    PyObject *func;
    PyObject *ids;
    PyObject *key;
} CVPeriodicSlate;


static void CVPeriodicSlate_dealloc(CVPeriodicSlate *self)
{
    Py_DECREF(self->weak_actor);

    if (self->coro) {
        cv_dealloc_coroutine(self->coro);
    }
    if (self->func) {
        Py_CLEAR(self->func);
    }
    if (self->key) {
        Py_DECREF(self->key);
    }
    PyObject_Del( (PyObject *)self );
}


static PyTypeObject CVPeriodicSlate_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "CVPeriodicSlate",                      /* tp_name */
    sizeof(CVPeriodicSlate),      /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)CVPeriodicSlate_dealloc,  /* tp_dealloc */
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
    slate_doc_string,                   /* tp_doc */
};

#define CVPeriodicSlate_Check(op) PyObject_TypeCheck(op, &CVPeriodicSlate_Type)


static CVPeriodicSlate* new_cv_preiodic_state(PyObject *actor, PyObject *ids)
{
    CVPeriodicSlate *slate = PyObject_New(CVPeriodicSlate, &CVPeriodicSlate_Type);

    if (slate == NULL) {
        return NULL;
    }
    else {
        PyObject *weak_actor = PyWeakref_NewRef(actor, NULL);
        
        if (weak_actor == NULL) {
            PyObject_Del((PyObject *)slate);
            return NULL;
        }
        slate->ids = ids;
        slate->actor = actor;
        slate->weak_actor = weak_actor;
        slate->q = &(((CVObject *)actor)->cvprocesses);
    }
    slate->coro = NULL;
    slate->func = NULL;
    slate->key = NULL;
    return slate;
}


static Uint32 cv_slate_periodic_callback(Uint32 interval, void *param)
{
    PyObject *v;
    CVCoroutine *coro;
    CVPeriodicSlate *this = (CVPeriodicSlate *)param;
    
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
    else if (cv_schedule_period(coro, this->ids, this->key) < 0) {
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
    int i;
    CVPeriodicSlate *timer_data = new_cv_preiodic_state(self, ids);

    if (timer_data == NULL) {
        return -1;
    }
    else if (PyDict_GetItemString(ids, name) != NULL) {
        PyErr_Format(PyExc_RuntimeError, "'%s' is already a scheduled periodic event.", name);
        return -1;
    }
    else {
        PyObject *func;
        PyObject *meth;

        meth = PyObject_GetAttrString(self, name);

        if (meth == NULL) {
            PyErr_Format(PyExc_AttributeError, "'%s' has no event named '%s'.", PYOBJECT_NAME(self), name);
            Py_DECREF((PyObject *)timer_data);
            return -1;
        }
        else if (!PyMethod_Check(meth)) {
            Py_DECREF(meth);
            PyErr_Format(PyExc_AttributeError, "'%s' has no event named '%s'.", PYOBJECT_NAME(self), name);
            Py_DECREF((PyObject *)timer_data);
            return -1;
        }
        func = PyMethod_Function(meth);
        timer_data->func = func;
        Py_INCREF(func);
        Py_DECREF(meth);
    }
    i = PyDict_SetItemString(ids, name, (PyObject *)timer_data);
    Py_DECREF((PyObject *)timer_data);// Ref-count now == 1

    if (i < 0) {
        return -1;
    }
    else {
        PyObject *key = PyString_FromString(name);

        if (key == NULL) {
            PyDict_DelItemString(ids, name);
            return -1;
        }
        else {
            CVCoroutine *coro = cv_create_coroutine(self);

            if (coro == NULL) {
                Py_DECREF(key);
                PyDict_DelItemString(ids, name);
                return -1;
            }
            else if (cv_schedule_period(coro, ids, key) < 0) {
                cv_dealloc_coroutine(coro);
                Py_DECREF(key);
                PyDict_DelItemString(ids, name);
                return -1;
            }
            Py_INCREF(ids);
            Py_INCREF(key);
            timer_data->key = key;
            timer_data->coro = coro;
        }
    }
    //timer_data->actor = self;
    //timer_data->q = &(((CVObject *)self)->cvprocesses);
    //timer_data->weak_actor = weak_actor;
    timer_data->timer_id = SDL_AddTimer(delay, &cv_slate_periodic_callback, (void *)timer_data);

    if (!timer_data->timer_id) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        PyDict_DelItemString(ids, name); //Will take care of the rest.
        return -1;
    }
    return 0;
}


/* Scheduled Once Event Dispatch ****************************************************************************************** */
typedef struct _cvslatedata {
    CVCoroutine super;
    SDL_TimerID timer_id;
} CVSlateData;


typedef struct _cvslate {
    PyObject_HEAD
    PyObject *weak_actor;
    PyObject *func;
    CVObjectQ coro_q;
    CVObjectQ *actor_q;
} CVSlate;


static void CVSlate_dealloc(CVSlate *self)
{
    Py_DECREF(self->weak_actor);
    Py_DECREF(self->ids);

    if (self->coro) {
        cv_dealloc_coroutine(self->coro);
    }
    if (self->func) {
        Py_CLEAR(self->func);
    }
    if (self->key) {
        Py_DECREF(self->key);
    }
    PyObject_Del( (PyObject *)self );
}


static PyTypeObject CVSlate_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "CVSlate",                      /* tp_name */
    sizeof(CVSlate),      /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)CVSlate_dealloc,  /* tp_dealloc */
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
    slate_doc_string,                   /* tp_doc */
};


static CVSlateData* new_cv_slate_data(PyObject *actor)
{/* Taken from `cv_create_coroutine` */
    CVStack self_stack;
    CVCoroState self_state;
    CVSlateData *self = (CVSlateData *)PyMem_Malloc(_cvslate);

    if (self == NULL) {
        return PyErr_NoMemory();
    }
    else {
        PyObject *state_actor_ptr = PyWeakref_NewRef(actor, NULL);

        if (state_actor_ptr == NULL) {
            PyMem_Free(self);
            return NULL;
        }
        self_state.actor_ptr = state_actor_ptr;
        self_state.parent = NULL;
    }
    cv_init_costack(&self_stack);
    self->super.stack = self_stack;
    self->super.state = self_state;
    self->timer_id = 0;
    return self;
}


static CVSlate* new_cv_slate(PyObject *actor, PyObject *ids)
{
    CVSlate *slate = PyObject_New(CVSlate, &CVSlate_Type);

    if (slate == NULL) {
        return NULL;
    }
    else {
        PyObject *weak_actor = PyWeakref_NewRef(actor, NULL);

        if (weak_actor == NULL) {
            PyObject_Del((PyObject *)slate);
            return NULL;
        }
        slate->weak_actor = weak_actor;
    }
    slate->ids = ids;
    slate->func = NULL;
    cv_init_object_queue(&slate->coro_q);
    slate->actor_q = &(((CVObject *)actor)->cvprocesses);
    return slate;
}


static Uint32 cv_slate_callback(Uint32 interval, void *param)
{
    PyObject *v;
    CVCoroutine *coro;
    CVSlate *the = (CVSlate *)param;

    CV_BEGIN_DENY_THREADS

    /* Pass the interval's value to the coroutine */
    v = Py_BuildValue("(OI)", the->weak_actor, interval);

    if (v == NULL) {
        CV_COLLAPSE_THREAD();
    }
    coro = cv_object_queue_pop(&the->coro_q);
    SDL_assert(coro != NULL);

    if ((cv_spawn(coro, the->func, v, NULL) < 0) || (cv_push_event(the->weak_actor, CV_DISPATCHED_EVENT, 0) < 0) || (cv_object_queue_push(the->actor_q, coro) < 0)) {
        Py_DECREF(v);
        CV_COLLAPSE_THREAD();
    }

    CV_END_DENY_THREADS

    return 0;
}


static int cv_schedule_once(PyObject *self, const char *name, Uint32 delay, PyObject *ids)
{
    CVSlateData *slate_data = cv_create_slate_data(self);

    if (slate_data == NULL) {
        PyDict_DelItemString(ids, name);
        return -1;
    }
    else {
        CVSlate *timer_data;

        {
            PyObject *something = PyDict_GetItemString(ids, name);
        
            if (something == NULL) {
                timer_data = new_cv_slate(self, ids);
        
                if (timer_data == NULL) {
                    cv_dealloc_slate_data(slate_data);
                    return -1;
                }
                else if (PyDict_SetItemString(ids, name, (PyObject *)timer_data)) {
                    Py_DECREF((PyObject *)timer_data);
                    cv_dealloc_slate_data(slate_data);
                    return -1;
                }
                else {
                    PyObject *meth;

                    Py_DECREF((PyObject *)timer_data); // Borrow the reference from `ids` going forward
                    meth = PyObject_GetAttrString(self, name);
    
                    if (meth == NULL) {
                        PyErr_Format(PyExc_AttributeError, "'%s' has no event named '%s'.", PYOBJECT_NAME(self), name);
                        PyDict_DelItemString(ids, name);
                        cv_dealloc_slate_data(slate_data);
                        return -1;
                    }
                    else if (!PyMethod_Check(meth)) {
                        Py_DECREF(meth);
                        PyErr_Format(PyExc_AttributeError, "'%s' has no event named '%s'.", PYOBJECT_NAME(self), name);
                        PyDict_DelItemString(ids, name);
                        cv_dealloc_slate_data(slate_data);
                        return -1;
                    }
                    timer_data->func = PyMethod_Function(meth);
                    Py_INCREF(timer_data->func);
                    Py_DECREF(meth);
                }
                timer_data->key = PyString_FromString(name);

                if (timer_data->key == NULL) {
                    PyDict_DelItemString(ids, name);
                    cv_dealloc_slate_data(slate_data);
                    return -1;
                }
            }
            else if (CVPeriodicSlate_Check(something)) {
                PyErr_Format(PyExc_RuntimeError, "'%s' is already a scheduled periodic event.", name);
                cv_dealloc_slate_data(slate_data);
                return -1;
            }
            else {
                timer_data = (CVSlate *)something;
            }
        }
        if ((cv_schedule_period((CVCoroutine *)slate_data, ids, timer_data->key) < 0) || (cv_object_queue_push(timer_data->coro_q, (CVCoroutine *)slate_data) < 0)) {
            cv_dealloc_slate_data(slate_data);
            return -1;
        }
    }

    slate_data->timer_id = SDL_AddTimer(delay, &cv_slate_callback, (void *)timer_data);

    if (!slate_data->timer_id) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        cv_dealloc_slate_data(slate_data);
        return -1;
    }
    return 0;
}


static PyObject* EventDispatcher_schedule(CVEventDispatcher self, PyObject *args, PyObject *kwargs)
{
    int repeat = 0;
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
        else if (delay < 0) {
            PyErr_SetString(PyExc_ValueError, "'delay' must be greater than, or equal to, 0 miliseconds.");
            return NULL;
        }
    }

    if (!repeat && (cv_schedule_once((PyObject *)self, name, delay, self->timer_ids) < 0)) {
        return NULL;
    }
    else if (cv_schedule_interval((PyObject *)self, name, delay, self->time_ids) < 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}
