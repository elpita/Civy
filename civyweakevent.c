typdedef struct _cvweakevent *CVWeakEvent;
struct _cvweakevent {
    int id;
    PyObject *obj;
    PyObject *name;
}


static int CVWeakEvent_init(CVWeakEvent self, PyObject *args, PyObject *kwds)
{
    PyObject *callback;
    static char *kwargs[] = {"callback", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwargs, &callback)) {
        return -1;
    }
    else if (PyFunction_Check(callback)) {
        self->obj = callback;
    }
    else {
        PyObject *obj = PyObject_GetAttrString(callback, "__func__");
        
        if (obj == NULL) {
            return -1;
        }
        self->obj = obj;
    }
    self->id = PyObject_Hash(callback);

    if (id < 0) {
        Py_DECREF(self->obj);
        return -1;
    }
}
