from cpython.ref cimport PyObject

cdef extern from "Python.h":
    PyObject* PyWeakref_NewRef(PyObject *ob, PyObject *callback)
    PyObject* PyWeakref_GetObject(PyObject *ref)
    PyObject* PyObject_CallMethodObjArgs(PyObject *o, PyObject *name, ..., NULL)
    
cdef class WeakMethod(object):
    cdef int id
    cdef object _obj
    cdef str _func
