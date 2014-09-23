#ifndef civyobject_included
#define civyobject_included
#include "civyprocess.h"
#include "SDL.h"

Uint32 DISPATCHED_EVENT = SDL_RegisterEvents(1);
typedef struct _cvobject *CVObject;

static PyObject* CVObject_spawn(CVObject self, PyObject *callback, PyObject *args);
static PyObject* CVObject_exec(PyObject *self);
static PyObject* CVObject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs);
static int CVObject_init(CVObject self, PyObject *args, PyObject *kwargs);
static void CVObject_fork(CVObject self, PyObject *callback, PyObject *data);
static void CVObject_dealloc(CVObject self);
#endif
