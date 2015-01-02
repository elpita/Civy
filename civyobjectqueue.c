#include "civyobjectqueue.h"
#define Q_IS_EMPTY(q) (q->head == NULL)

typedef struct _cvobjectqueueentry {
    CVCoroutine routine;
    CVObjectQEntry *previous;
    CVObjectQEntry *next;
} CVObjectQEntry;

struct _cvobjectqueue {
    CVObjectQEntry *head;
    CVObjectQEntry *tail;
};


static void cv_init_object_queue(CVObjectQ self)
{
    self->head = self->tail = NULL;
}


static int cv_object_queue_push(CVObjectQ self, CVCoroutine coro)
{
    /* We're gonna borrow some of python's internals for a little bit */
    CVObjectQEntry *new_entry = (CVObjectQEntry *)PyObject_Malloc(sizeof(struct _cvobjectqueueentry));

    if (new_entry == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    new_entry->routine = coro;
    new_entry->previous = self->tail;
    new_entry->next = NULL;

    switch(self->tail == NULL) {
        case 0:
            self->tail = self->tail->next = new_entry;
            break;
        case 1:
            self->head = self->tail = new_entry;
            break;
    }
    return 0;
}


static CVCoroutine cv_object_queue_pop(CVObjectQ self)
{
    CVCoroutine coro;
    CVObjectQEntry *entry;

    if (Q_IS_EMPTY(self)) {
        return NULL;
    }
    entry = self->head;
    coro = entry->routine;
    self->head = entry->next;

    switch(self->head == NULL) {
        case 0:
            self->head->previous = NULL;
            break;
        case 1:
            self->tail = self->head;
            break;
    }
    PyObject_Free(entry);
    return coro;
}


static void cv_dealloc_object_queue(CVObjectQ self)
{
    CVCoroutine c;

    while ((c = cv_object_queue_pop(self)) != NULL) {
        cv_dealloc_coroutine(c);
    }
}
