cdef struct context:
    cdef PyGreenlet *self
    cdef Context *previous
    cdef Context *next
ctypedef context Context

cdef class EventDeque(object):
    cdef Context *head
    cdef Context *tail
    #cdef Context* create_entry(EventDeque self, PyGreenlet *event) except -1
    cdef urgent(EventDeque self, PyGreenlet *event) except *
    cdef push(EventDeque self, PyGreenlet *event) except *
    cdef PyGreenlet* pop(EventDeque self) except -1
    cdef PyGreenlet* undo(EventDeque self) except -1
