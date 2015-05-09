cdef class WeakList(list):
    cdef object __weakref__ 
    cdef object _remove
