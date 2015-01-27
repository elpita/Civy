#include "civycoroutine.h"


typedef struct _cvcorostate {
    CVCoroutine *parent;
    PyObject *actor_ptr;
} CVCoroState;


typedef struct _cvcoroutine {
    CVStack stack;
    CVCoroState state;
} CVCoroutine;


static CVCoroutine* cv_create_coroutine(PyObject *actor)
{
    CVStack self_stack;
    CVCoroState self_state;
    CVCoroutine *self = (CVCoroutine *)PyMem_Malloc(sizeof(CVCoroutine));

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
    self->stack = self_stack;
    self->state = self_state;
    return self;
}


static void kill_cvcoroutine(CVCoroutine *c)
{
    cv_dealloc_costack(&c->stack);
    Py_DECREF((c->state).actor_ptr);
    PyMem_Free(c);
}


static void cv_dealloc_coroutine(CVCoroutine *self)
{
    CVCoroutine *p, *c;

    for (p = (self->state).parent; p != NULL; p = c) {
        c = (p->state).parent;
        kill_cvcoroutine(p);
    }
    kill_cvcoroutine(self);
}


static int cv_check_continuation(CVCoroState *c)
{
    if (c->parent == NULL) {
        return 1;
    }
    else if is_dead(c->actor_ptr) {
        return 0;
    }
    else {
        CVCoroState *p = &(c->parent->state);
        return cv_check_continuation(p);
    }
}
