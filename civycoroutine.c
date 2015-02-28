#include "civycoroutine.h"


typedef struct _cvcorostate {
    PyObject *actor_ptr;
    CVCoroutine *parent;
} CVCoroState;


typedef struct _cvcoroutine {
    CVStack stack;
    CVCoroState state;
} CVCoroutine;


static CVCoroutine* cv_create_coroutine(PyObject *actor)
{
    CVStack self_stack;
    CVCoroState self_state;
    CVCoroutine *self = (CVCoroutine *)PyMem_Malloc(sizeof(_cvcoroutine));

    if (self == NULL) {
        return PyErr_NoMemory();
    }
    else {
        PyObject *state_actor_ptr = actor->weak_ref;

        self_state.actor_ptr = state_actor_ptr;
        Py_INCREF(state_actor_ptr);
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
    Py_DECREF( ((CVCoroState *)c)->actor_ptr );
    PyMem_Free(c);
}


static void cv_dealloc_coroutine(CVCoroutine *self)
{
    CVCoroutine *p, *c;

    for (p = ((CVCoroState *)c)->actor_ptr; p != NULL; p = c ) {
        c = ((CVCoroState *)p)->parent;
        kill_cvcoroutine(p);
    }
    kill_cvcoroutine(self);
}


static int cv_check_continuation(CVCoroState *c)
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
            CVCoroState p_state = parent->state;

            return cv_check_continuation(&p_state);
        }
    }
}
