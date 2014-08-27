cdef struct context:
    cdef PyGreenlet *self
    cdef Context *previous
    cdef Context *next
ctypedef context Context

cdef class EventDeque(object):
    cdef Context *head
    cdef Context *tail
    cdef void urgent(EventDeque self, PyGreenlet *event) except *
    cdef void push(EventDeque self, PyGreenlet *event) except *
    cdef PyGreenlet* pop(EventDeque self) except -1
    cdef PyGreenlet* undo(EventDeque self) except -1