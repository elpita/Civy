cdef extern from "Python.h":
    object PyWeakref_NewRef(object ob, object callback)
    object PyWeakref_GET_OBJECT(object ref)
    int PySequence_Check(object o)
    int PySlice_Check(object ob)
    void Py_XINCREF(object o)


cdef class WeakList(list):
    cdef object _remove
