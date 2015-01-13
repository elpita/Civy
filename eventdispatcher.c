static int str_endswith(PyObject *key, const char *suffix)
{
    Py_ssize_t len;
    const char *str;

    if (!PyString_Check(key)) {
        return 0;
    }
    len = PyString_GET_SIZE(key);
    
    if (len <= 6) {
        return 0;
    }
    str = PyString_AS_STRING(key);
    return (strncmp(str + (len - 6), suffix, 6) == 0);


static PyObject* EventDispatcherType_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    char *name;
    Py_ssize_t pos = 0;
    PyObject *key, *value, *weak_value;

    while (PyDict_Next(kwds, &pos, &key, &value)) {
        if (str_endswith(key, "_event") && PyFunction_Check(value)) {
            weak_value = PyWeakref_NewProxy(value, NULL);

            if (weak_value == NULL) {
                return NULL;
            }
            else if (PyDict_SetItem(some_dict, key, weak_value) < 0) {
                Py_DECREF(weak_value);
                return NULL;
            }
            Py_DECREF(weak_value);
        }
    }
    return type->tp_base->tp_new(type, args, kwds);
