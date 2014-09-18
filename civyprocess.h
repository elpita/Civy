#ifndef civycontext_included
#define civycontext_included

#include "q.h"
#include "greenlet.h"


typedef QueueEntry _CVProcess *CVProcess;


static PyObject* CVProcess_loop(PyObject *capsule);
static CVProcess CVProcess_new(PyObject *event_handler);
static int CVProcess_dealloc(CVProcess self);
static int CVProcess_push_thread(CVProcess self, PyGreenlet *data);
static PyGreenlet* CVProcess_pop_thread(CVProcess self);

static void **IMPORT_civyprocess = NULL;

#define DOT_CVPROCESS_NEW                       0
#define DOT_CVPROCESS_DEALLOC                   1
#define DOT_CVPROCESS_PUSH_THREAD               2
#define DOT_CVPROCESS_POP_THREAD                3

#define civyprocess_dot_CVProcess_new           (*(_CVProcess *)IMPORT_civyprocess[DOT_CVPROCESS_NEW])
#define civyprocess_dot_CVProcess_dealloc       (*(int)IMPORT_civyprocess[DOT_CVPROCESS_DEALLOC])
#define civyprocess_dot_CVProcess_push_thread   (*(int *)IMPORT_civyprocess[DOT_CVPROCESS_PUSH_THREAD])
#define civyprocess_dot_CVProcess_pop_thread    (*(PyGreenlet *)IMPORT_civyprocess[DOT_CVPROCESS_POP_THREAD])
#endif