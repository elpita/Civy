#ifndef eventhandlers_included
#define eventhandlers_included
#include "civyobject.h"


typedef struct _eventdispatcher *EventDispatcher;
typedef struct _callbackhandler *CallbackHandler;
typedef struct _cvproperty *CVProperty;


static int CVObject_schedule(PyObject *self, PyObject *callback, PyObject *data);
static int CVProperty_dispatch(PyObject *observers, PyObject *obj, PyObject *args);
static PyObject* CVProperty_descr_get(CVProperty self, PyObject *obj, PyObject *type);
static int CVProperty_descr_set(CVProperty self, EventDispatcher obj, PyObject *new_value);


#endif
