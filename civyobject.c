#include "civyobject.h"
#include "structmember.h"
#include <assert.h>
#define CVObject_push_process(self, new_entry)	q_dot_Queue_push(self->cvprocesses, (QEntry *)new_entry)
#define CVObject_pop_process(self)				(_CVProcess *)q_dot_Queue_pop(self->cvprocesses)
#define sentinel_doc "If you can read this, you're probably looking at the wrong object."

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
    0,                         /*ob_size*/  
    "sentinel",                /*tp_name*/
    sizeof(SentinelObject),       /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    0,                         /*tp_flags*/
    sentinel_doc,              /* tp_doc */
    };


#define Sentinel_Check(op)	PyObject_TypeCheck(op, &SentinelObjectType)


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


static PyObject* CVObject_spawn(CVObject self, PyObject *callback, PyObject *args)
{
	PyGreenlet *event = PyGreenlet_New(callback, NULL);
	PyGreenlet *live_thread = PyGreenlet_GetCurrent();

	if (_current == NULL) { /* i.e., a fresh process */
		assert(Q_IS_EMPTY(self->cvprocesses)); //You shouldn't be able to reach this function directly;
		CVProcess new_processs = civyprocess_dot_CVProcess_new(self);
		if (new_process == NULL) {
			return NULL;
		}

		if (CVProcess_push_threads(new_processs, live_thread) < 0) {
			return NULL;
		}
		CVObject_push_process(self, new_process);
		args = PyGreenlet_Switch(self->exec, args, NULL);
	}
	
	SentinelObject *sentinel =  PyObject_New(SentinelObject, &SentinelObjectType);
	if (sentinel == NULL) {
		return NULL;
	}

	sentinel->data = args;
	Py_INCREF(args);

	if (_current->handler <> self) {
		CVProcess child_process = civyprocess_dot_CVProcess_new(self);

		if (child_process == NULL) {
			Py_XDECREF(args);
			return NULL;
		}
		child_process->parent = _current;
		_current = child_process;
	}

	if ((CVProcess_push_threads(_current, live_thread) < 0) || (CVProcess_push_threads(_current, event) < 0)) {
		return NULL;
	}
	return PyGreenlet_Switch(self->exec, (PyObject *)sentinel, NULL);
}


void CV_join(PyObject* _target, PyObject *args, Uint32 event_type)
{
	PyObject* target = PyWeakref_NewRef(PyObject *_target, NULL);
	Py_INCREF(args);

	SDL_Event event;
	SDL_zero(event);
	event.type = event_type;
	event.user.code = 0;
	event.user.data1 = &target;
	event.user.data2 = &args;
	SDL_PushEvent(&event);

	/* if (SDL_PushEvent(&event) < 0) {
		Py_DECREF(target);
		Py_DECREF(args);

		if (PyErr_WarnEx(NULL, SDL_GetError(), 1) < 0) {
			return -1;
		}
		return 0;
	}
	return 1; */
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
                        switch(civyprocess_dot_CVProcess_dealloc(process)) {
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
                                        Py_DECREF(data->data);
                                        Py_DECREF(data);
                                        break;
                                    default:
                                        switch(process->parent == NULL) {
                                            case 0:
                                                parent = process->parent;
                                                CVObject_push_process(parent->handler, parent);
                                                CV_join(self, data, DISPATCHED_EVENT);
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
    	/* DANGER ZONE: How to check for stack overflow...? */
        civyprocess_dot_CVProcess_dealloc(CVObject_pop_process(self->cvprocesses));
    }
    q_dot_Queue_dealloc(self->cvprocesses);
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
