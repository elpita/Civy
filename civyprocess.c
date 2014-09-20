#include "civyprocess.h"


QueueEntry _CVProcess {
    PyObject *handler;
    PyGreenlet *loop;
    CVProcess parent;
    Q pipeline;
    };

typedef QueueEntry CVContext {
    PyGreenlet *cvthread;
    };


static PyObject* CVProcess_loop(PyObject *capsule)
{
    CVProcess self = (_CVProcess *)PyCapsule_GetPointer(capsule, NULL);
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
    CVProcess process = (_CVProcess *)malloc(sizeof(_CVProcess));

    if (process == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    process->pipeline = q_dot_Queue_new();
    
    if (process->pipeline == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    process->loop = PyGreenlet_New(CVProcess_loop, NULL);
    PyObject *capsule = PyCapsule_New((void *)process, NULL, NULL);
    PyGreenlet_Switch(process->loop, capsule);
    process->handler = event_handler;
    process->parent = NULL;
    return process;
}


static int CVProcess_dealloc(CVProcess self)
/* CVProcess method: Kill All */
{
    if (self == NULL) {
        return 0;
    }
    if (self->parent <> NULL) {
        if (Py_EnterRecursiveCall(" in CVProcess deallocation.") <> 0) || (CVProcess_dealloc(self->parent) == -1) {
            return -1;
        }
        Py_LeaveRecursiveCall();
    }
    while (!Q_IS_EMPTY(self->pipeline)) {
        Py_CLEAR(CVThreads_pop(self->pipeline));
    }
    self->handler == NULL;
    Py_CLEAR(self->loop);
    q_dot_Queue_dealloc(self->pipeline);
    free(self);
    return 0;
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
    q_dot_Queue_prepend(self->pipeline, (QEntry *)new_entry);
    Py_INCREF(cvthread);
    return PyGreenlet_SetParent(cvthread, self->loop);
}


static PyGreenlet* CVProcess_pop_thread(CVProcess self)
/* CVProcess method: Pop greenlets from mini-queue */
{
    CVContext *entry = (CVContext *)q_dot_Queue_pop(self->pipeline);

    if (entry == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    PyGreenlet *result = entry->cvthread;
    free(entry);
    Py_DECREF(result);
    return result;
}


IMPORT_civyprocess[] = {
    (void *)CVProcess_new,
    (void *)CVProcess_dealloc,
    (void *)CVProcess_push_thread,
    (void *)CVProcess_pop_thread
    };
