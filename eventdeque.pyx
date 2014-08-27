cdef class EventDeque(object):
    slots = ('head', 'tail')

    def __cinit__(self):
        self.head = <Context*>malloc(sizeof(Context))
        self.tail = <Context*>malloc(sizeof(Context))
        self.head = self.tail = NULL

    def __dealloc__(self):
        while (self.head <> NULL):
            Py_DECREF(self.pop())
        free(self.head)
        free(self.tail)

    cdef void push(EventDeque self, PyGreenlet *event):
        cdef Context *new_entry = malloc(sizeof(Context))
        new_entry.self = event

        new_entry.previous = self.tail
        new_entry.next = NULL

        if self.tail == NULL:
            self.head = self.tail = new_entry
        else:
            self.tail.next = self.tail = new_entry

    cdef PyGreenlet* pop(EventDeque self):
        if self.head == NULL:
            return NULL
        cdef Context *entry = self.head
        cdef PyGreenlet *result = entry.data
        self.head = entry.next

        if self.head == NULL:
            self.tail = self.head
        else:
            self.head.previous = NULL

        free(entry)
        return result

    cdef void urgent(EventDeque self, PyGreenlet *event):
        cdef Context *new_entry = malloc(sizeof(QueueEntry))
        new_entry.self = event

        new_entry.previous = NULL
        new_entry.next = self.head

        if self.head == NULL:
            self.head = self.tail = new_entry
        else:
            self.head.previous = self.head = new_entry

    cdef PyGreenlet* undo(EventDeque self):
        if self.head == NULL:
            return NULL
        cdef Context *entry = self.tail
        cdef PyGreenlet *result = entry.data
        self.tail = entry.previous

        if self.tail == NULL:
            self.head = self.tail
        else:
            self.tail.next = NULL

        free(entry)
        return result

    def bint __bool__(self):
        return self.head <> NULL
