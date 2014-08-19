cdef class WeakMethod(object):
    __slots__ = ('_obj', '_func', 'id')

    def __cinit__(self, object method):
        self.id = method.__hash__()
        self._func = method.__name__
        cdef object _obj = method.im_self if method.__self__ is not None else method.im_class
        self._obj = <object>PyWeakref_NewRef(<PyObject*>_obj, <PyObject*>None)

    def __call__(self, *args):
        if not self.is_dead:
            return <object>(PyObject_CallMethodObjArgs(PyWeakref_GetObject(<PyObject*>self._obj), <PyObject*>self._func, <PyObject*>args))

    def __richcmp__(self, object other, int op):
        if op == 2:
            return self.id == other.__hash__()
        elif op == 3:
            return self.id <> other.__hash__()

    property is_dead:
        def bint __get__(WeakMethod self):
            return <object>(PyWeakref_GetObject(<PyObject*>self._obj)) is None
