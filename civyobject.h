#include <Python.h>
#include "greenlet.h"

typedef struct {
    PyObject_HEAD
    PyObject *store_event;
    PyObject *load_event;
    PyGreenlet *main_loop;   /* "Class Method" */
} CVObject;


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static int CVObject_init(CVObject *self, PyObject *args, PyObject *kwargs);
static void CVObject_dealloc(CVObject *self);

static PyTypeObject CVObject_Type;

#ifndef CVOBJECTMODINIT_FUNC
#define CVOBJECTMODINIT_FUNC void
#endif
