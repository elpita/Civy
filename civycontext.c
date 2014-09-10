#include "civycontext.h"

static PyObject* CVContext_loop(PyObject *capsule)
{
    CVContext *self = (CVContext *)PyCapsule_GetPointer(capsule, NULL);
    Py_DECREF(capsule);
    PyObject *args = PyGreenlet_Switch( (PyGreenlet_GetCurrent())->parent, NULL, NULL );
    PyGreenlet *g;

    while ( (!Q_is_empty(self->cvthreads)) || (g <> NULL) )
    {
        g = CVThreads_pop(self);
        args = PyGreenlet_Switch(g, args, NULL);
        Py_XDECREF(g);
        }

    Py_XDECREF(args);
    return NULL;
    }


CVContext* CVContext_new(PyObject *event_handler)
/* To be called from `CVObject` */
{
    CVContext *context = (CVContext *)malloc(sizeof(CVContext));
    context->loop = PyGreenlet_New(CVContext_loop, NULL);
    PyObject *capsule = PyCapsule_New((void *)context, NULL, NULL);
    PyGreenlet_Switch(context->loop, capsule);
    Py_XDECREF(capsule);
    context->handler = PyWeakref_NewRef(event_handler, NULL);
    return context;
    }


void CVContext_dealloc(CVContext *self)
/* CVContext method: Kill All */
{
    if (self->parent <> NULL)
    {
        CVContext_dealloc(self->parent);
        }

    while (!Q_is_empty(self->cvthreads))
    {
        Py_CLEAR(CVThreads_pop(self->cvthreads));
        }

    Py_CLEAR(self->handler);
    Py_CLEAR(self->loop);
    free(self->cvthreads);
    free(self);
    }


int CVThreads_push(CVContext *self, PyGreenlet *data)
/* CVContext method: Push greenlets onto mini-queue */
{
    whatever *new_entry = (whatever *)malloc(sizeof(whatever));
    
    if (new_entry == NULL)
    {
        return -1;
        }

    new_entry->cvthread = data;
    Q_prepend(self->cvthreads, (QEntry *)new_entry);
    return 0;
    }


PyGreenlet* CVThreads_pop(CVContext *self)
/* CVContext method: Pop greenlets from mini-queue */
{
    whatever *entry = (whatever *)Q_pop(self->cvthreads);

    if (entry == NULL)
    {
        return NULL;
        }

    PyGreenlet *result = entry->cvthread;
    free(entry);
    return result;
    }
