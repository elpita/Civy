#include "civycoroutine.h"

struct _cvcoroutine {
    struct _cvstack stack;
    CVCoroutine parent;
    PyObject *actor_ptr;
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
