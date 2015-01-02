#include "civyobject.h"

struct _cvobject {
    PyObject_HEAD
    struct _cvobjectqueue cvprocesses;
    PyObject *in_weakreflist;
};


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject self = (struct _cvobject *)type->tp_alloc(type, 0);

    if (self == NULL) {
        return PyErr_NoMemory();
    }
    self->in_weakreflist = NULL;
    cv_init_object_queue(&self->cvprocesses);
    return (PyObject *)self;
}


static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs) {
    if (main_loop == NULL) { /* To be set when the main loop starts */
        PyErr_SetString(PyExc_TypeError, "The App's Main Loop must be started first.");
        return -1;
    }
    return 0;
}


static void CVObject_dealloc(CVObject self) {
    switch(self->in_weakreflist != NULL) {
        case 1:
            PyObject_ClearWeakRefs((PyObject *)self);
        default:
            cv_dealloc_object_queue(&self->cvprocesses);
            self->ob_type->tp_free( (PyObject *)self );
            break;
    }
}
