#include "civyobject.h"
#include "structmember.h"
#define CVObject_push_process(self, new_entry)  q_dot_Queue_push(self->cvprocesses, (QEntry *)new_entry)
#define CVObject_pop_process(self)              (_CVProcess *)q_dot_Queue_pop(self->cvprocesses)

struct _cvobject {
    PyObject_HEAD
    Q cvprocesses;
    PyGreenlet *exec;
    };


static int check_process(CVProcess process)
/* If the process chain is broken anywhere (i.e., through killing a `CVObject`), there's no reason to execute the process */
/* Returns 0 (fail), 1(pass), or -1(error) */
{
    if (process == NULL) {
        return 0;
    }
    int i = 1;

    if (process->parent <> NULL) 
    {
        switch(Py_EnterRecursiveCall(" in CVprocess checking.")) {
            case 0:
                i = check_process(process->parent);

                switch(i) {
                    case -1:
                        return -1;
                    default:
                        Py_LeaveRecursiveCall();
                        break;
                }
                break;
            default:
                return -1;
        }
    }
    return (i && PyGreenlet_ACTIVE(process->handler));
}


static PyObject* CVObject_exec(PyObject *self)
{
    PyObject *data = PyGreenlet_Switch( (PyGreenlet_GetCurrent())->parent, NULL, NULL );
    CVprocess *process, *parent;

    while (1)
    { /* Yeah, that's right. I'm using `switch`es, and i don't care. */
        switch(Q_IS_EMPTY(self->cvprocesses)) {
            case 1:
                break; //only the `swtich` sequence
            default:
                process = CVObject_pop_process(self);

                switch(check_process(process)) {
                    case -1:
                        return -1;
                    case 0:
                        switch(civyprocess_dot_CVProcess_dealloc(process)) {
                            case -1:
                                return -1;
                        }
                    default:
                        data = PyGreenlet_Switch(process->spawn, data, NULL);
                        
                        switch(data == NULL) {
                            case 1:
                                break;
                            default:
                                switch(Wait_Check(data)) {
                                    case 1:
                                        CVObject_push_process(self, process);
                                        /* Schedule_SDL(data.data) */
                                        Py_DECREF(data);
                                        break;
                                    default:
                                        switch(Fork_Check(data)) {
                                            case 1:
                                                Py_DECREF(data);
                                                break;
                                            default:
                                                switch(process->parent == NULL) {
                                                    case 0:
                                                        parent = process->parent;
                                                        CVObject_push_process(parent->handler, parent);
                                                        /* Schedule_SDL(data) */
                                                        process->parent = NULL;
                            
                                                        switch(civyprocess_dot_CVProcess_dealloc(process)) {
                                                            case -1:
                                                                return -1;
                                                        }
                                                        break;
                                                }
                                                break;
                                        }
                                        break;
                                }
                            break;
                        }
                    break;
                }
            break;
        }
        //Py_XDECREF(data);
        data = PyGreenlet_Switch(self->main_loop, data, NULL);
    }
}


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject self = (_cvobject *)type->tp_alloc(type, 0);

    if (self == NULL) {
        Py_XDECREF(self);
        return NULL;
    }

    self->cvprocesses = q_dot_Queue_new();

    if (self->cvprocesses == NULL) {
        Py_XDECREF(self);
        return NULL;
    }

    PyGreenlet *process_loop = PyGreenlet_New(CVObject_exec, NULL);

    if (process_loop == NULL) {
        Py_DECREF(self);
        return NULL;
    }

    PyGreenlet_Switch(process_loop, (PyObject *)self, NULL);
    self->exec = process_loop; //proxy?
    return (PyObject *)self;
}


static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs)
{
    if (self->main_loop == NULL) { /* Fix this */
        PyErr_SetString(PyExc_TypeError, "The App Main Loop must be started first.");
        return -1;
    }
    return 0
}


static void CVObject_dealloc(CVObject self)
{
    while (!Q_IS_EMPTY(self->cvprocesses)) {
        civyprocess_dot_CVProcess_dealloc(CVObject_pop_process(self->cvprocesses));
    }
    q_dot_Queue_dealloc(self->cvprocesses);
    Py_DECREF(self->exec);
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


PyMODINIT_FUNC initcivyobject(void)
{
    CVObject.main_loop = (PyGreenlet *)PyMem_Malloc(sizeof(PyGreenlet)); /*Fix this */
    CVObject.main_loop = NULL; /*Fix this */

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
