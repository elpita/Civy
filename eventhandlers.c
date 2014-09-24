whatever
{
    const char *name;
    name = PyString_AsString(key);
    if (name[3] == "on_") && (PyString_Size(key) > 3) && (Py_TYPE(value) == PyFunction_Type) {
        PyObject *new_value = EventProperty(value);
        if (new_value == NULL) {
            return -1;
        }
        else if (PyDict_SetItem(self->cv, key, new_value) < 0) {
            Py_DECREF(new_value);
            return -1;
        }
        Py_DECREF(new_value);
    }

    else if (Py_TYPE(value) == Property) {
        if (PyDict_SetItem(self->cv, key, value) < 0) {
            return -1;
        }
    }
    


static PyObject *
EventDispatcherType_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    const char *name;
    Py_ssize_t pos = 0;
    PyObject *key, *value;

    while (PyDict_Next(kwds, &pos, &key, &value)) {
	name = PyString_AsString(key);
        if (name[3] == "on_") && (PyString_Size(key) > 3) && (Py_TYPE(value) == PyFunction_Type){
            PyObject *new_value = EventProperty(value);
            if (new_value == NULL)
                return -1;
            if (PyDict_SetItem(kwds, key, new_value) < 0) {
                Py_DECREF(new_value);
                return -1;
            }
            Py_DECREF(new_value);
        }
    }

    return type->tp_base->tp_new(type, args, kwds);
}

static PyTypeObject EventDispatcherType_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "eventdispatcher",                             /* tp_name */
    sizeof(PyType_Type),                           /* tp_basicsize */
    0,                                             /* tp_itemsize */
    0,                                             /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_compare */
    0,                                             /* tp_repr */
    0,                                             /* tp_as_number */
    0,                                             /* tp_as_sequence */
    0,                                             /* tp_as_mapping */
    0,                                             /* tp_hash */
    0,                                             /* tp_call */
    0,                                             /* tp_str */
    0,                                             /* tp_getattro */
    0,                                             /* tp_setattro */
    0,                                             /* tp_as_buffer */
    0,                                             /* tp_flags */
    0,                                             /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    0,                                             /* tp_iter */
    0,                                             /* tp_iternext */
    0,                                             /* tp_methods */
    0,                                             /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                                             /* tp_init */
    0,                                             /* tp_alloc */
    EventDispatcherType_new,                       /* tp_new */
};
