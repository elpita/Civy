#include "civyactorqueue.h"
#define Q_IS_EMPTY(q) (q->head == NULL)

typedef struct _cvactorqueueentry {
    CVCoroutine routine;
    CVActorQEntry *previous;
    CVActorQEntry *next;
} CVActorQEntry;

struct _cvactorqueue {
    CVActorQEntry *head;
    CVActorQEntry *tail;
};


static int cv_actor_queue_push(CVActorQ self, CVCoroutine coro)
{
    /* We're gonna borrow some of python's internals for a little bit */
    CVActorQEntry *new_entry = (CVActorQEntry *)PyObject_Malloc(sizeof(struct _cvactorqueueentry));

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


static CVCoroutine cv_actor_queue_pop(CVActorQ self)
{
    CVCoroutine coro;
    CVActorQEntry *entry;

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
