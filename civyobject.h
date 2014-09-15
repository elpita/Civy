#ifndef civyobject_included
#define civyobject_included

#include "civyprocess.h"

typedef struct _cvobject *CVObject;
static PyTypeObject CVObject_Type;


static int check_process(CVProcess process);
static PyObject* CVObject_exec(PyObject *self);
static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs);
static void CVObject_dealloc(CVObject self);



#endif
