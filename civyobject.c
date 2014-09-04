#include "civyobject.h"
#include "structmember.h"
int init_event_greenlets(PyGreenlet *g, CVObject *self);


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject *self = (CVObject *)type->tp_alloc(type, 0);

    if (self <> NULL) {
        PyGreenlet *store_event = PyGreenlet_New(dispatch_global, NULL);
        PyGreenlet *load_event = PyGreenlet_New(eventloop_global, NULL);

        if ((store_event == NULL) || (load_event == NULL)) {
            Py_DECREF(self);
            return NULL;
            }

        /* Either this is a really good idea, or a really stupid one...Let's find out! */
        self->store_event = PyWeakref_NewProxy((PyObject *)store_event, NULL);
        self->load_event = PyWeakref_NewProxy((PyObject *)load_event, NULL);
        }

    return (PyObject *)self;
    }


int init_event_greenlets(PyGreenlet *g, CVObject *self)
{
    PyGreenlet *c = PyGreenlet_GetCurrent();
    PyObject *args = PyTuple_Pack(2, self, c);
    
    if args == NULL {
        Py_DECREF(c);
        Py_DECRERF(self);
        Py_DECREF(args);
        return -1;
        }

    PyGreenlet_Switch(g, args, NULL);
    Py_DECREF(c);
    Py_DECRERF(self);
    Py_DECREF(args);
    return 0;
    }


static int CVObject_init(CVObject *self, PyObject *args, PyObject *kwargs)
{
    PyGreenlet *main = self->main_loop;
    if (main == NULL) {
        PyErr_SetString(PyExc_TypeError, "The App Main Loop must be started first.");
        return -1;
        }

    /* Finish initializing the event greenlets */
    if (init_event_greenlets((PyGreenlet *)(self->store_event), self) < 0)
        return -1;
    PyGreenlet_SetParent( (PyGreenlet *)(self->store_event), main );
    
    if (init_event_greenlets((PyGreenlet *)(self->load_event), self) < 0)
        return -1;
    PyGreenlet_SetParent( (PyGreenlet *)(self->load_event), main );
    return 0
    }


static void CVObject_dealloc(CVObject *self)
{
    /* Either this was a really good idea, or a really stupid one. */
    Py_XDECREF( PyWeakref_GetObject(self->store_event) );
    Py_XDECREF( PyWeakref_GetObject(self->load_event) );
    self->ob_type->tp_free( (PyObject *)self );
    }


static PyTypeObject CVObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "CVObject",                                 /*tp_name*/
    sizeof(CVObject),                           /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)CVObject_dealloc,               /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash */
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "The Control ",                             /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
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


static PyMethodDef module_methods[] = {
    {NULL}
    };


CVOBJECTMOODINIT_FUNC initcivyobject(void)
{
    CVObject.main_loop = (PyGreenlet *)PyMem_Malloc(sizeof(PyGreenlet));
    CVObject.main_loop = NULL;

    if (PyType_Ready(&CVObject_Type) == 0) {
        PyObject *m = Py_InitModule3("civyobject", module_methods, "The heart of Civy.");

        if (m <> NULL) {
            Py_INCREF(&CVObject_Type);
            PyModule_AddObject(m, "CVObject", (PyObject *)&CVObject_Type);
            PyGreenlet_Import();
            }
        }
    }


/* TODO: Incorporate `SDL_Event`
         Figure out how to free `main_loop` memory
         Create the functions
         Create queues
         Error checking for overriding `__new__`
         */
