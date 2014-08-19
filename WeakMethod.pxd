from cpython.ref cimport PyObject

cdef extern from "Python.h":
    object PyWeakref_NewRef(PyObject *ob, PyObject *callback)
    PyObject* PyWeakref_GetObject(PyObject *ref)
    object PyObject_GetAttr(PyObject *o, PyObject *name)
    void Py_XINCREF(PyObject *o)
    
cdef class WeakMethod(object):
    cdef int id
    cdef object _obj
    cdef str _func
    cdef PyObject* _get_object(self, object x)
