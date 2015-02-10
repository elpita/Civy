cdef inline object _get_object(object x):
    x = PyWeakref_GET_OBJECT(x)
    Py_XINCREF(x)
    return x


cdef class WeakCallback:
    __slots__ = ('obj')
    cdef object _obj

    def __cinit__(self, WeakList obj):
        self._obj = PyWeakref_NewRef(obj, None)

    def __call__(self, object item):
        cdef object weak_list = PyWeakref_GET_OBJECT(self._obj)

        if (weak_list == None):
            return
        while list.__contains__(weak_list, item):
            list.remove(weak_list, item)


cdef inline object _get_ref(object x, object self):
    return PyWeakref_NewRef(x, self._remove)


cdef class WeakList(list):

    def __cinit__(self, *args):
        self._remove = WeakCallback(self)

    def __init__(self, items=None):
        cdef object x
        items = items or []
        super(WeakList, self).__init__((_get_ref(x, self) for x in items))

    def __add__(self, object items):
        if not isinstance(items, WeakList):
            raise TypeError("can only concatenate WeakList (not '{!s}') to WeakList".format(type(items)))
        return super(WeakList, self).__add__(items)

    def __contains__(self, object item):
        return super(WeakList, self).__contains__(_get_ref(item, self))

    def __delitem__(self, object item):
        super(WeakList, self).__delitem__(_get_ref(item, self))

    def __getitem__(self, object i):
        if not PySlice_Check(i):
            return _get_object(super(WeakList, self).__getitem__(i))
        cdef object x
        cdef object gen = (_get_object(x) for x in super(WeakList, self).__getitem__(i))
        return list(gen)

    def __getslice__(self, Py_ssize_t i, Py_ssize_t j):
        cdef slice s = slice(i, j, None)
        return self.__getitem__(s)

    def __iadd__(self, object items):
        if isinstance(items, WeakList):
            return super(WeakList, self).__iadd__(items)
        cdef object x
        return super(WeakList, self).__iadd__((_get_ref(x, self) for x in items))

    def __iter__(self):
        cdef object x
        for x in super(WeakList, self).__iter__():
            yield _get_object(x)

    def __repr__(self):
        return "WeakList({!r})".format(list(self))

    def __reversed__(self):
        cdef object x
        for x in super(WeakList, self).__reversed__():
            yield _get_object(x)

    def __setitem__(self, object i, object item):
        if not PySlice_Check(i):
            super(WeakList, self).__setitem__(i, _get_ref(item, self))
            return
        cdef object x
        cdef object gen = (_get_ref(x, self) for x in item)
        super(WeakList, self).__setitem__(i, gen)

    def __setslice__(self, Py_ssize_t i, Py_ssize_t j, object items):
        cdef slice s = slice(i, j, None)
        if not PySequence_Check(items):
            items = (items,)
        self.__setitem__(s, items)

    def append(self, object item):
        super(WeakList, self).append(_get_ref(item, self))

    def count(self, object item):
        return super(WeakList, self).count(_get_ref(item, self))

    def extend(self, object items):
        cdef object x
        super(WeakList, self).extend((_get_ref(x, self) for x in items))

    def index(self, object item):
        return super(WeakList, self).index(_get_ref(item, self))

    def insert(self, Py_ssize_t i, object item):
        super(WeakList, self).insert(i, _get_ref(item, self))

    def pop(self, Py_ssize_t i=-1):
        return _get_object(super(WeakList, self).pop(i))

    def remove(self, object item):
        super(WeakList, self).remove(_get_ref(item, self))
