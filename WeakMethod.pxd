from cpython.ref cimport PyObject

cdef extern from "Python.h":
    PyObject* PyWeakref_NewRef(PyObject *ob, PyObject *callback)
    PyObject* PyWeakref_GetObject(PyObject *ref)
    PyObject* PyObject_GetAttr(PyObject *o, PyObject *name)
    void Py_XINCREF(PyObject *o)
    
cdef class WeakMethod(object):
    cdef int id
    cdef object _obj
    cdef str _func
    cdef PyObject* _get_object(self, PyObject *x)
