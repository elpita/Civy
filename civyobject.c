#include "civyobject.h"
#include <assert.h>
#define CVObject_push_process(self, new_entry) Queue_push(self->cvprocesses, (struct _queueentry *)new_entry)
#define CVObject_pop_process(self) (struct _cvprocess *)Queue_pop(self->cvprocesses)
#define sentinel_doc "If you can read this, you're probably looking at the wrong object."
static void Sentinel_dealloc(SentinelObject *self);

CVObject const _current = NULL;


struct _cvobject {
    PyObject_HEAD
    Q cvprocesses;
    PyGreenlet *exec;
    PyObject *in_weakreflist;
    };
static PyTypeObject CVObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "CVObject",                                 /*tp_name*/
    sizeof(_cvobject),                          /*tp_basicsize*/
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
    offsetof(_cvobject, in_weakreflist),        /* tp_weaklistoffset */
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
    Py_XDECREF(self->data);
    PyObject_Del( (PyObject *)self );
}


static int check_cvprocess(CVProcess process)
/* If the process chain is broken anywhere (i.e., through killing a `CVObject`), there's no reason to execute the process */
/* Returns 1(pass), 0(fail), or -1(error). */
{
    switch((process == NULL) || !(PyGreenlet_ACTIVE(((struct _cvobject *)process)->handler->exec))) {
        case 1:
            return 0;
        default:
            switch(process->parent == NULL) {
                case 1:
                    return 1;
                case 0:
                    switch(Py_EnterRecursiveCall(" in CVProcess checking.")) {
                        case 0:
                            int i = check_cvprocess(process->parent);
            
                            switch(i) {
                                case -1:
                                    return -1;
                                default:
                                    Py_LeaveRecursiveCall();
                                    break;
                            }
                            return i;
                        default:
                            return -1;
                    }
            }
    }
}


static int CV_join(PyObject *_target, PyObject *args, Uint32 event_type)
{
    PyObject *target = PyWeakref_NewRef(PyObject *_target, NULL);

    if (target == NULL) {
        return -1;
    }

    Py_INCREF(args);
    SDL_Event event;

    SDL_zero(event);
    event.type = event_type;
    event.user.code = 0;
    event.user.data1 = &target;
    event.user.data2 = &args;

    if (SDL_PushEvent(&event) < 0) {
        Py_DECREF(target);
        Py_DECREF(args);
        return -1;
    }
    return 0;
}


static int CVObject_schedule(CVObject self, PyObject *callback, PyObject *data)
{
    CVProcess process = CVProcess_new(self);

    if (process == NULL) {
        return -1;
    }
    PyGreenlet *g = PyGreenlet_New(callback, NULL);
    
    if (g == NULL) {
        CVProcess_dealloc(process);
        return -1;
    }
    else if (CVProcess_push_thread(process, g) < 0) {
        Py_DECREF(g);
        CVProcess_dealloc(process);
        return -1;
    }
    CVObject_push_process(self, process);
    return CV_join(self, data, DISPATCHED_EVENT);
}


static PyObject* CVObject_spawn(CVObject self, PyObject *callback, PyObject *args)
{
    PyGreenlet *live_thread = PyGreenlet_GetCurrent();

    if (_current == NULL) { //i.e., a fresh process
        assert(Q_IS_EMPTY(self->cvprocesses)); //You shouldn't be able to reach this function directly
        CVProcess new_process = CVProcess_new(self);

        if (new_process == NULL) {
            return NULL;
        }
        else if (CVProcess_push_thread(new_process, live_thread) < 0) {
            CVProcess_dealloc(new_process);
            return NULL;
        }
        CVObject_push_process(self, new_process);
        args = PyGreenlet_Switch(self->exec, args, NULL);
    }
    PyGreenlet *event = PyGreenlet_New(callback, NULL);

    if ((event == NULL) || (CVProcess_push_thread(_current, live_thread) < 0)) {
        return NULL;
    }
    SentinelObject *sentinel = PyObject_New(SentinelObject, &SentinelObjectType);

    if (sentinel == NULL) {
        Py_DECREF(event);
        return NULL;
    }
    sentinel->data = args;
    Py_INCREF(args);

    if (_current->handler <> self) {
        CVProcess child_process = CVProcess_new(self);

        if (child_process == NULL) {
            Py_DECREF(sentinel);
            Py_DECREF(event);
            return NULL;
        }
        child_process->parent = _current;
        _current = child_process;
    }
    if (CVProcess_push_thread(_current, event) < 0) {
        CVProcess_dealloc(child_process);
        Py_DECREF(sentinel);
        Py_DECREF(event);
        return NULL;
    }
    return PyGreenlet_Switch(self->exec, (PyObject *)sentinel, NULL);
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

                switch(check_cvprocess(process)) {
                    case -1:
                        return NULL;
                    case 0:
                        CVProcess_dealloc(process);
                        break;
                    default:
                        _current = process;
                        data = PyGreenlet_Switch(process->loop, data, NULL);

                        switch(data == NULL) {
                            case 1:
                                return NULL;
                            default:
                                switch(Sentinel_Check(data)) {
                                    case 1:
                                        CVObject_push_process(self, process);

                                        switch(CV_join(self, data->data, DISPATCHED_EVENT)) {
                                            default:
                                                Py_DECREF(data);
                                            case -1:
                                                return NULL;
                                            case 0:
                                                break;
                                        }
                                        break;

                                    default:
                                        switch(process->parent == NULL) {
                                            case 0:
                                                parent = process->parent;
                                                CVObject_push_process(parent->handler, parent);

                                                switch(CV_join(parent->handler, data, DISPATCHED_EVENT)) {
                                                    case -1:
                                                        return NULL;
                                                    default:
                                                        process->parent = NULL;
                                                        CVProcess_dealloc(process);
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
        _current = NULL;
        data = PyGreenlet_Switch(main_loop, data, NULL);
    }
    return data;
}


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CVObject self = (struct _cvobject *)type->tp_alloc(type, 0);

    if (self == NULL) {
        return NULL;
    }
    self->in_weakreflist = NULL;
    self->cvprocesses = Queue_new();

    if (self->cvprocesses == NULL) {
        Py_DECREF(self);
        return NULL;
    }
    PyGreenlet *process_loop = PyGreenlet_New(CVObject_exec, NULL);

    if (process_loop == NULL) {
        Queue_dealloc(self->cvprocesses);
        Py_DECREF(self);
        return NULL;
    }
    PyObject *_ = PyGreenlet_Switch(process_loop, (PyObject *)self, NULL);
    self->exec = process_loop;
    return (PyObject *)self;
}


static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs)
{
    if (main_loop == NULL) { /* To be set when the main loop starts */
        PyErr_SetString(PyExc_TypeError, "The App Main Loop must be started first.");
        return -1;
    }
    return 0
}


static void CVObject_dealloc(CVObject self)
{
    if (self->in_weakreflist <> NULL) {
        PyObject_ClearWeakRefs((PyObject *)self);
    }
    while (!Q_IS_EMPTY(self->cvprocesses)) {
        CVProcess_dealloc(CVObject_pop_process(self->cvprocesses));
    }
    Queue_dealloc(self->cvprocesses);
    Py_XDECREF(self->exec);
    self->ob_type->tp_free( (PyObject *)self );
}


/*
static int _initcivyobject(void *type)
{
    assert(DISPATCHED_EVENT <> ((Uint32)-1));
    _cvobject.app = NULL;
    SentinelObjectType.tp_new = PyType_GenericNew;
    
    if (PyType_Ready(&SentinelObjectType) < 0) {
        return -1;
    }
    
    (*((PyTypeObject *)type)).tp_base = &CVObject_Type;
    
    if (PyType_Ready(&CVObject_Type) < 0) {
        return -1;
    }

    // Must also init `CVObject_Type`

    return 0;
}
*/
