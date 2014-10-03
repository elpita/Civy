#include "civyprocess.h"


struct _cvprocess {
    QEntry super;
    PyGreenlet *loop;
    Q pipeline;
    PyObject *handler;
    CVProcess parent;
    };


typedef struct CVContext {
    QEntry super;
    PyGreenlet *cvthread;
    };


static PyObject* CVProcess_loop(PyObject *capsule)
{
    CVProcess self = (CVProcess)PyCapsule_GetPointer(capsule, NULL);
    Py_DECREF(capsule);
    PyObject *args = PyGreenlet_Switch( (PyGreenlet_GetCurrent())->parent, NULL, NULL );
    PyGreenlet *g;

    while (!Q_IS_EMPTY(self->pipeline)) {
        g = CVThreads_pop(self);

        if (g == NULL) {
            break;
        }
        args = PyGreenlet_Switch(g, args, NULL);
        Py_XDECREF(g);
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
    process->pipeline = Queue_new();
    
    if (process->pipeline == NULL) {
        free(process);
        PyErr_NoMemory();
        return NULL;
    }
    process->loop = PyGreenlet_New(CVProcess_loop, NULL);
    PyObject *capsule = PyCapsule_New((void *)process, NULL, NULL);
    PyObject *_ = PyGreenlet_Switch(process->loop, capsule);
    Py_DECREF(_);
    process->handler = event_handler;
    process->parent = NULL;
    return process;
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
            CVProcess_dealloc(parent);
        }
    }
    while (!Q_IS_EMPTY(self->pipeline)) {
        Py_CLEAR(CVThreads_pop(self->pipeline));
    }
    self->handler == NULL;
    Py_CLEAR(self->loop);
    Queue_dealloc(self->pipeline);
    free(self);
}


static int CVProcess_push_thread(CVProcess self, PyGreenlet *cvthread)
/* CVProcess method: Push greenlets onto mini-queue */
{
    CVContext *new_entry = (CVContext *)malloc(sizeof(CVContext));
    
    if (new_entry == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    new_entry->cvthread = cvthread;
    Py_INCREF(cvthread);
    Queue_prepend(self->pipeline, (struct _queueentry *)new_entry);
    return PyGreenlet_SetParent(cvthread, self->loop);
}


static PyGreenlet* CVProcess_pop_thread(CVProcess self)
/* CVProcess method: Pop greenlets from mini-queue */
{
    CVContext *entry = (CVContext *)Queue_pop(self->pipeline);

    if (entry == NULL) {
        return NULL;
    }
    PyGreenlet *result = entry->cvthread;
    Py_DECREF(result);
    free(entry);
    return result;
}
