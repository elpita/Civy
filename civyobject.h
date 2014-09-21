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
static void CVObject_dealloc(CVObject self);

static int _initcivyobject(void *type)
static void **IMPORT_civyprocess = NULL;

#define DOT_CVOBJECT_SPAWN                  0
#define DOT__initcivyobject                 1

#define civyobject_dot_CVObject_spawn       (*(_cvobject *)IMPORT_civyobject[DOT_CVOBJECT_SPAWN])
#define _INITCIVYOBJECT                     (*(int)IMPORT_civyobject[DOT__initcivyobject])

#endif
