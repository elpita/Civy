#include "civycontext.h"

CVContext* CVContext_new(PyObject *blah)
{
    CVContext *context = (CVContext *)malloc(sizeof(CVContext));
    context->loop = PyGreenlet_New(global_whatever, NULL);
    //PyGreenlet_Switch(context->loop, (PyCapsule(context)))
    context->interrupt_handler = PyWeakref_NewProxy(blah, NULL);
    return context;
    }


void CVContext_dealloc(CVContext *self)
/* CVContext method: Kill All */
{
    if (self->parent_chain <> NULL)
    {
        CVContext_dealloc(self->parent_chain);
        }

    while (!Q_is_empty(self->cvthreads))
    {
        Py_CLEAR(Q_pop(self->cvthreads));
        }

    Py_CLEAR(self->interrupt_handler);
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
