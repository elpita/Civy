#include <Python.h>
#include "greenlet.h"

typedef struct {
    PyObject_HEAD
    PyObject *store_event;
    PyObject *load_event;
    PyGreenlet *main_loop;   /* "Class Method" */
} CVObject;
