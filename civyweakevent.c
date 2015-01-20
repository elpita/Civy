typdedef struct _cvweakevent *CVWeakEvent;
struct _cvweakevent {
    PyObject *obj;
    int id;
}


static int CVWeakEvent_init(CVWeakEvent self, PyObject *args, PyObject *kwds)
{
    PyObject *callback;
    long int id;
    static char *kwargs[] = {"callback", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwargs, &callback)) {
        return -1;
    }
    else if (PyFunction_Check(callback)) {
        self->obj = callback;
        Py_INCREF(callback);
    }
    else {
        PyObject *obj = PyObject_GetAttrString(callback, "__func__");
        
        if (obj == NULL) {
            return -1;
        }
        self->obj = obj;
    }
    id = PyObject_Hash(callback);

    if (id < 0) {
        Py_DECREF(self->obj);
        return -1;
    }
    self->id = id;
    return 0;
}


static PyObject* CVWeakEvent_call(CVWeakEvent self, PyObject *args, PyObject *kwds)
{
    s
}


static void CVWeakEvent_dealloc(CVWeakEvent self)
{
    Py_DECREF(self->obj);
    self->ob_type->tp_free((PyObject *)self);
}
