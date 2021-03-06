typedef struct _cvinputobject {
    PyObject_HEAD
    unsigned int timestamp;
    PyObject *in_wrl;
} CVInputObject;


static PyObject* CVInputObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CVInputObject *self = (CVInputObject *)type->tp_alloc(type, 0);

    if (self == NULL) {
        return NULL;
    }
    self->in_wrl = NULL;
    self->timestamp = 0;
    return (PyObject *)self;
}


static PyObject* CVInputObject_gettimestamp(CVInputObject *self, void *)
{
    PyObject *pdt;

    {
        PyObject *value = Py_BuildValue("(I)", self->timestamp);

        if (value = NULL) {
            return NULL;
        }
        pdt = PyDateTime_FromTimestamp(value);
        Py_DECREF(value);
    }

    if (pdt == NULL) {
        return NULL;
    }
    else {
        int h = PyDateTime_DATE_GET_HOUR(pdt);
        int m = PyDateTime_DATE_GET_MINUTE(pdt);
        int s = PyDateTime_DATE_GET_SECOND(pdt);
        int u = PyDateTime_DATE_GET_MICROSECOND(pdt);

        Py_DECREF(pdt);
        return PyTime_FromTime(h, m, s, u);
    }
}


static int CVInputObject_settimestamp(CVInputObject *self, PyObject *, void *)
{
    PyErr_SetString(PyExc_TypeError, "'timestamp' attribute is read-only");
    return -1;
}


static PyGetSetDef CVInputObject_getseters[] = {
    {"timestamp", (getter)CVInputObject_gettimestamp, (setter)CVInputObject_settimestamp, "time stamp", NULL}, {NULL}
};


static void CVInputObject_dealloc(CVInputObject *self)
{
    switch(self->in_wrl != NULL) {
        case 1:
            PyObject_ClearWeakRefs((PyObject *)self);
        default:
            self->ob_type->tp_free((PyObject *)self);
            break;
    }
}


static PyTypeObject CVInputObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /* ob_size */
    "CVInputObject",                            /* tp_name */
    sizeof(_cvinputobject),                     /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)CVInputObject_dealloc,          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Input Object Base Type",                   /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    offsetof(_cvinputobject, in_wrl),           /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    CVInputObject_getseters,                    /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    CVInputObject_new,                          /* tp_new */
};



/*********************************** DROPFILE ****************************************/ 

