#include "civyobject.h"
#define cv_object_is_dead(o) !o->alive


struct _cvobject {
    PyObject_HEAD
    int alive;
    struct _cvobjectqueue cvprocesses;
    PyObject *in_weakreflist;
};

static PyTypeObject CVObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /* ob_size */
    "CVObject",                                 /* tp_name */
    sizeof(struct _cvobject),                   /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)CVObject_dealloc,               /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "The Control ",                             /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    offsetof(struct _cvobject, in_weakreflist), /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)CVObject_init,                    /* tp_init */
    0,                                          /* tp_alloc */
    CVObject_new,                               /* tp_new */
};


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject self = (struct _cvobject *)type->tp_alloc(type, 0);

    if (self == NULL) {
        return NULL;
    }
    self->in_weakreflist = NULL;
    cv_init_object_queue(&self->cvprocesses);
    self->alive = 1;
    return (PyObject *)self;
}


static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs)
{
    if (main_loop == NULL) { /* To be set when the main loop starts */
        PyErr_SetString(PyExc_TypeError, "The App's Main Loop must be started first.");
        return -1;
    }
    return 0;
}


static void CVObject_dealloc(CVObject self)
{
    switch(self->in_weakreflist != NULL) {
        case 1:
            PyObject_ClearWeakRefs((PyObject *)self);
        default:
            self->alive = 0;
            cv_dealloc_object_queue(&self->cvprocesses);
            self->ob_type->tp_free( (PyObject *)self );
            break;
    }
}
