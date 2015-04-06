typedef struct _cvcorostate {
    PyObject *actor_ptr;
    CVCoroutine *parent;
} CVCoroState;


typedef struct _cvcoroutine {
    _cvcorostate state;
    _cvstack stack;
} CVCoroutine;


static CVCoroutine* cv_create_coroutine(PyObject *actor_ptr, CVCoroutine *parent)
{
    CVCoroState self_state;
    CVCoroutine *self = (CVCoroutine *)PyMem_Malloc(sizeof(_cvcoroutine));

    if (self == NULL) {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyErr_NoMemory();
        PyGILState_Release(gstate);
        return NULL;
    }
    else {
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_INCREF(actor_ptr);
        PyGILState_Release(gstate);
    }

    self_state.actor_ptr = actor_ptr;
    self_state.parent = parent;
    self->state = self_state;
    cv_init_costack(self->stack);
    return self;
}


static void kill_cvcoroutine(CVCoroutine *c)
{
    {
        CVCostack stack = c->stack;
        cv_dealloc_costack(&stack);
    }{
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyObject *actor_ptr = ((CVCoroState *)c)->actor_ptr;

        Py_DECREF(actor_ptr);
        PyGILState_Release(gstate);
    }
    PyMem_Free(c);
}


static void cv_dealloc_coroutine(CVCoroutine *self)
{
    CVCoroutine *p;
    CVCoroState *state = (CVCoroState *)self;

    for (p = state->parent; p != NULL; p = state->parent) {
        state = ((CVCoroState *)p);
        kill_cvcoroutine(p);
    }
    kill_cvcoroutine(self);
}


static int cv_check_coroutine(CVCoroState *c)
{
    if is_dead(c->actor_ptr) {
        return 0;
    }
    else {
        CVCoroutine *parent = c->parent;

        if (parent == NULL) {
            return 1;
        }
        else {
            CVCoroState *p_state = (CVCoroState *)parent;

            return cv_check_continuation(p_state);
        }
    }
}
