#include "civyobject.h"
#include "structmember.h"
#include <assert.h>
#define CVObject_push_process(self, new_entry) Queue_push(self->cvprocesses, (QEntry *)new_entry)
#define CVObject_pop_process(self) (_CVProcess *)Queue_pop(self->cvprocesses)
#define sentinel_doc "If you can read this, you're probably looking at the wrong object."
static void Sentinel_dealloc(SentinelObject *self);

CVObject const _current = NULL;


struct _cvobject {
    PyObject_HEAD
    Q cvprocesses;
    PyGreenlet *exec;
    };
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


typedef struct SentinelObject {
    PyObject_HEAD
    PyObject *data;
    };
static PyTypeObject SentinelObjectType = {
    PyObject_HEAD_INIT(NULL)
    0,                             /*ob_size*/  
    "sentinel",                    /*tp_name*/
    sizeof(SentinelObject),        /*tp_basicsize*/
    0,                             /*tp_itemsize*/
    (destructor)Sentinel_dealloc,  /*tp_dealloc*/
    0,                             /*tp_print*/
    0,                             /*tp_getattr*/
    0,                             /*tp_setattr*/
    0,                             /*tp_compare*/
    0,                             /*tp_repr*/
    0,                             /*tp_as_number*/
    0,                             /*tp_as_sequence*/
    0,                             /*tp_as_mapping*/
    0,                             /*tp_hash */
    0,                             /*tp_call*/
    0,                             /*tp_str*/
    0,                             /*tp_getattro*/
    0,                             /*tp_setattro*/
    0,                             /*tp_as_buffer*/
    0,                             /*tp_flags*/
    sentinel_doc,                  /* tp_doc */
    };


#define Sentinel_Check(op)	PyObject_TypeCheck(op, &SentinelObjectType)

static void Sentinel_dealloc(SentinelObject *self)
{
    Py_DECREF(self->data);
    self->ob_type->tp_free( (PyObject *)self );
}


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
        switch(Py_EnterRecursiveCall(" in CVProcess checking.")) {
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
    CVProcess process, parent;

    while (1)
    { /* Yeah, that's right. I'm using `switch`es, and i don't care. */
        switch(Q_IS_EMPTY(self->cvprocesses)) {
            case 1:
                break;
            default:
                process = CVObject_pop_process(self);
                _current = process;

                switch(check_process(process)) {
                    case -1:
                        return NULL;
                    case 0:
                        switch(CVProcess_dealloc(process)) {
                            case -1:
                                return NULL;
                            case 0:
                                break;
                        }
                    default:
                        data = PyGreenlet_Switch(process->loop, data, NULL);
                        
                        switch(data == NULL) {
                            case 1:
                                return NULL;
                            default:
                                switch(Sentinel_Check(data)) {
                                    case 1:
                                        CVObject_push_process(self, process);
                                        CV_join(self, data->data, DISPATCHED_EVENT);
                                        Py_DECREF(data);
                                        break;
                                    default:
                                        switch(process->parent == NULL) {
                                            case 0:
                                                parent = process->parent;
                                                CVObject_push_process(parent->handler, parent);
                                                CV_join(self, data, DISPATCHED_EVENT);
                                                process->parent = NULL;
                    
                                                switch(CVProcess_dealloc(process)) {
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
        //Py_XDECREF(data);
        _current = NULL;
        data = PyGreenlet_Switch(self->main_loop, data, NULL);
    }
    return data;
}


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject self = (_cvobject *)type->tp_alloc(type, 0);

    if (self == NULL) {
        Py_XDECREF(self);
        return NULL;
    }

    self->cvprocesses = Queue_new();

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
    if (self->app == NULL) { /* To be set when the main loop starts */
        PyErr_SetString(PyExc_TypeError, "The App Main Loop must be started first.");
        return -1;
    }
    return 0
}


static void CVObject_dealloc(CVObject self)
{
    while (!Q_IS_EMPTY(self->cvprocesses)) {
    	/* DANGER ZONE: How to check for stack overflow...? */
        CVProcess_dealloc(CVObject_pop_process(self->cvprocesses));
    }
    Queue_dealloc(self->cvprocesses);
    Py_DECREF(self->exec);
    self->ob_type->tp_free( (PyObject *)self );
}


static int _initcivyobject(void *type)
{
	/* Set the Main Loop? */
	assert(DISPATCHED_EVENT <> (Uint32)-1));

	SentinelObjectType.tp_new = PyType_GenericNew;

	if (PyType_Ready(&SentinelObjectType) < 0) {
		return -1;
	}
	(*((PyTypeObject*)type)).tp_base = &CVObject_Type; //reminder --> expecting address

	if (PyType_Ready(&CVObject_Type) < 0) {
		return -1;
	}
	return 0;
}
/*
	if (PyType_Ready(&CVObject_Type) == 0) {
		PyObject *m = Py_InitModule3("civyobject", module_methods, "The heart of Civy.");
		if (m <> NULL) {
			Py_INCREF(&CVObject_Type);
			PyModule_AddObject(m, "CVObject", (PyObject *)&CVObject_Type);
			PyGreenlet_Import();
		}
	}
}*/
