cdef class Context:
    cdef PyGreenlet *__self__
    cdef Context previous
    cdef Context next

cdef class EventDeque(object):
    cdef Context head
    cdef Context tail
    cdef bint push_head(self, PyGreenlet *event)
    cdef bint push_tail(self, PyGreenlet *event)
    cdef bint pop_head(self, PyGreenlet *event)
    cdef bint pop_tail(self, PyGreenlet *event)
