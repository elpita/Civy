cdef class WeakMethod(object):
    __slots__ = ('_obj', '_func', 'id')

    def __cinit__(self, object method):
        self.id = method.__hash__()
        self._func = method.__name__
        if method.__self__ is not None:
            # bound method
            self._obj = PyWeakref_NewRef(method.im_self, None)
        else:
            # unbound method
            self._obj = PyWeakref_NewRef(method.im_class, None)

    cdef PyObject* _get_object(self, object x):
        cdef PyObject *y = PyWeakref_GetObject(x)
        Py_XINCREF(y)
        return y

    def __call__(self, *args):
        return PyObject_GetAttr(self._get_object(self._obj), <PyObject*>self._func)(*args)

    def __richcmp__(self, object other, int op):
        if op == 2:
            return self.id == other.__hash__()
        elif op == 3:
            return self.id <> other.__hash__()

    property is_dead:
        def bint __get__(WeakMethod self):
            return <object>PyWeakref_GetObject(<PyObject*>self._obj) is None
