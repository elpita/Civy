cdef extern from "Python.h":
    object PyWeakref_NewRef(object ob, object callback)
    object PyWeakref_GET_OBJECT(object ref)
    int PySequence_Check(object o)
    int PySlice_Check(object ob)
    void Py_XINCREF(object o)


cdef inline object _get_object(object x):
    x = PyWeakref_GET_OBJECT(x)
    Py_XINCREF(x)
    return x


cdef inline object _get_ref(object x, WeakList self):
    return PyWeakref_NewRef(x, self._remove)


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


cdef class WeakList(list):

    def __cinit__(self, *args):
        self._remove = WeakCallback(self)

    def __init__(self, items=None):
        cdef object x
        items = items or []
        list.__init__(self, (_get_ref(x, self) for x in items))

    def __add__(self, object items):
        if not isinstance(items, WeakList):
            raise TypeError("can only concatenate WeakList (not '{!s}') to WeakList".format(type(items)))
        return list.__add__(self, items)

    def __contains__(self, object item):
        return list.__contains__(self, _get_ref(item, self))

    def __delitem__(self, object item):
        list.__delitem__(self, _get_ref(item, self))

    def __getitem__(self, object i):
        if not PySlice_Check(i):
            return _get_object(list.__getitem__(self, i))
        cdef object x
        cdef object gen = (_get_object(x) for x in list.__getitem__(self, i))
        return list(gen)

    def __getslice__(self, Py_ssize_t i, Py_ssize_t j):
        cdef slice s = slice(i, j, None)
        return self.__getitem__(s)

    def __iadd__(self, object items):
        if isinstance(items, WeakList):
            return list.__iadd__(self, items)
        cdef object x
        return list.__iadd__(self, (_get_ref(x, self) for x in items))

    def __iter__(self):
        cdef object x
        for x in list.__iter__(self):
            yield _get_object(x)

    def __repr__(self):
        return "WeakList({!r})".format(list(self))

    def __reversed__(self):
        cdef object x
        for x in list.__reversed__(self):
            yield _get_object(x)

    def __setitem__(self, object i, object item):
        if not PySlice_Check(i):
            list.__setitem__(self, i, _get_ref(item, self))
            return
        cdef object x
        cdef object gen = (_get_ref(x, self) for x in item)
        list.__setitem__(self, i, gen)

    def __setslice__(self, Py_ssize_t i, Py_ssize_t j, object items):
        cdef slice s = slice(i, j, None)

        if not PySequence_Check(items):
            items = (items,)
        self.__setitem__(s, items)

    def append(self, object item):
        list.append(self, _get_ref(item, self))

    def count(self, object item):
        return list.count(self, _get_ref(item, self))

    def extend(self, object items):
        cdef object x
        list.extend(self, (_get_ref(x, self) for x in items))

    def index(self, object item):
        return list.index(self, _get_ref(item, self))

    def insert(self, Py_ssize_t i, object item):
        list.insert(self, i, _get_ref(item, self))

    def pop(self, Py_ssize_t i=-1):
        return _get_object(list.pop(self, i))

    def remove(self, object item):
        list.remove(self, _get_ref(item, self))
