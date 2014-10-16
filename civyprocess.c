#include "civyprocess.h"


struct _cvprocess {
    _queueentry super;
    PyGreenlet *loop;
    _queue pipeline;
    CVProcess parent;
    PyObject *handler;
    };


typedef struct _cvcontext {
    _queueentry super;
    PyGreenlet *cvthread;
    };


static PyObject* CVProcess_loop(PyObject *capsule)
{
    Q pipeline = (_queue *)PyCapsule_GetPointer(capsule, NULL);
    Py_DECREF(capsule);
    PyObject *args = PyGreenlet_Switch( (PyGreenlet_GetCurrent())->parent, NULL, NULL );
    PyGreenlet *g;

    while (!Q_IS_EMPTY(pipeline)) {
        g = CVProcess_pop_thread(pipeline);

        if (g == NULL) {
            break;
        }
        args = PyGreenlet_Switch(g, args, NULL);
        Py_DECREF(g);
    }
    return args;
}


static CVProcess CVProcess_new(PyObject *event_handler)
/* To be called from `CVObject` */
{
    CVProcess process = (struct _cvprocess *)malloc(sizeof(struct _cvprocess));

    if (process == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    Queue_init(process->pipeline);
    process->loop = PyGreenlet_New(CVProcess_loop, NULL);

    if (process->loop == NULL) {
        free(process);
        PyErr_NoMemory();
        return NULL;
    }
    PyObject *capsule = PyCapsule_New((void *)(process->pipeline), NULL, NULL);
    
    if (capsule == NULL) {
        Py_CLEAR(process->loop);
        free(process);
        PyErr_NoMemory();
        return NULL;
    }
    PyObject *_ = PyGreenlet_Switch(process->loop, capsule);
    process->handler = event_handler;
    process->parent = NULL;
    return process;
}


static void kill_cvprocess(CVProcess p)
{
    while (!Q_IS_EMPTY(p->pipeline)) {
        Py_CLEAR(CVProcess_pop_thread(p->pipeline));
    }
    Py_CLEAR(p->loop);
    free(p);
}


static void CVProcess_dealloc(CVProcess self)
/* CVProcess method: Kill All */
{
    if (self == NULL) {
        return;
    }
    if (self->parent <> NULL) {
        CVProcess parent;

        for (parent = self->parent; parent <> NULL; parent = parent->parent) {
            kill_cvprocess(parent);
        }
    }
    kill_cvprocess(self);
}


static int CVProcess_push_thread(CVProcess self, PyGreenlet *cvthread)
/* CVProcess method: Push greenlets onto mini-queue */
{
    _cvcontext *new_entry = (_cvcontext *)malloc(sizeof(_cvcontext));
    
    if (new_entry == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    new_entry->cvthread = cvthread;
    Py_INCREF(cvthread);
    Queue_prepend(self->pipeline, (_queueentry *)new_entry);
    return PyGreenlet_SetParent(cvthread, self->loop);
}


static PyGreenlet* CVProcess_pop_thread(Q pipeline)
/* CVProcess method: Pop greenlets from mini-queue */
{
    _cvcontext *entry = (_cvcontext *)Queue_pop(pipeline);

    if (entry == NULL) {
        return NULL;
    }
    PyGreenlet *result = entry->cvthread;
    Py_DECREF(result);
    free(entry);
    return result;
}
