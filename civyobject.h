#ifndef CIVYOBJECT_INCLUDED
#define CIVYOBJECT_INCLUDED
#include "civyobjectqueue.h"

typedef struct _cvobject *CVObject;

static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs);
static void CVObject_dealloc(CVObject self);

#endif /* CIVYOBJECT_INCLUDED */
