#include "civycoroutine.h"


typedef struct _cvcorostate {
    CVCoroutine parent;
    PyObject *actor_ptr;
} CVCoroState;

struct _cvcoroutine {
    struct _cvstack stack;
    struct _cvcorostate state;
};


static CVCoroutine cv_create_coroutine(PyObject *actor)
{
    CVCoroutine self = (struct _cvcoroutine *)PyMem_Malloc(sizeof(struct _cvcoroutine));

    if (self == NULL) {
        return PyErr_NoMemory();
    }
    self->actor_ptr = PyWeakref_NewRef(actor, NULL);

    if (self->actor_ptr == NULL) {
        PyMem_Free(self);
        return NULL;
    }
    cv_init_costack(&self->stack);
    self->parent = NULL;
    return self;
}


static void kill_cvcoroutine(CVCoroutine c)
{
    cv_dealloc_costack(&c->stack);
    Py_DECREF(c->actor_ptr);
    PyMem_Free(c);
}


static void cv_dealloc_coroutine(CVCoroutine self)
{
    CVCoroutine p, c;

    for(p = self->parent; p != NULL; p = c) {
        c = p->parent
        kill_cvcoroutine(p);
    }
    kill_cvcoroutine(self);
}


static int cv_check_continuation(CVCoroState c)
{
    if (c->parent == NULL) {
        return 1;
    }
    else if is_dead(c->actor_ptr) {
        return 0;
    }
    else {
        return cv_check_continuation(&c->parent->state);
    }
}
