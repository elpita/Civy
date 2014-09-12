#ifndef civyobject_included
#define civyobject_included

#include "civycontext.h"

typedef struct {
    PyObject_HEAD
    Q *cvprocesses;
    PyGreenlet *exec;
    PyGreenlet *main_loop;   /* "Class Method" */
} CVObject;


static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static int CVObject_init(CVObject *self, PyObject *args, PyObject *kwargs);
static void CVObject_dealloc(CVObject *self);

static PyTypeObject CVObject_Type;

#ifndef CVOBJECTMODINIT_FUNC
#define CVOBJECTMODINIT_FUNC void
#endif
#endif
