#include "civyobject.h"
#include "structmember.h"


static PyObject* CVObject_process_loop(PyObject *self)
{
    Q *cvprocesses = self->cvprocesses;
    Py_DECREF(self);
    PyGreenlet_Switch( (PyGreenlet_GetCurrent())->parent, NULL, NULL );
    CVContext *context;
    
    while (1)
    {
        while (!Q_is_empty(self->cvthreads))
        {
            
            }
        }
    }


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject *self = (CVObject *)type->tp_alloc(type, 0);

    if (self == NULL)
    {
        Py_XDECREF(self);
        return NULL;
        }

    PyGreenlet *process_loop = PyGreenlet_New(CVObject_process_loop, NULL);

    if (process_loop == NULL)
    {
        Py_DECREF(self);
        return NULL;
        }

    PyGreenlet_Switch(process_loop, (PyObject *)self, NULL);
    self->process_loop = process_loop; //proxy?
    return (PyObject *)self;
    }


static int CVObject_init(CVObject *self, PyObject *args, PyObject *kwargs)
{
    if (self->main_loop == NULL) 
    {
        PyErr_SetString(PyExc_TypeError, "The App Main Loop must be started first.");
        return -1;
        }

    return 0
    }


static void CVObject_dealloc(CVObject *self)
{
    while (!Q_is_empty(self->cvprocesses))
    {
        CVContext_dealloc(CVObject_pop_process(self->cvprocesses));
        }

    free(self->cvprocesses);
    Py_DECREF(self->process_loop);
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
