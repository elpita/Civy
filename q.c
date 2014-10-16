/* Special thanks to Simon Howard, C Algorithms library */

#include <stdlib.h>
#include "q.h"

/* Structures */
struct _queueentry {
    QEntry previous;
    QEntry next;
    };

typedef struct _queue {
    QEntry head;
    QEntry tail;
    };


/* Functions */
static void Queue_push(Q self, QEntry new_entry)
{
    new_entry->previous = self->tail;
    new_entry->next = NULL;

    switch(self->tail == NULL) {
        case 1:
            self->head = self->tail = new_entry;
            break;
        case 0:
            self->tail = self->tail->next = new_entry;
            break;
    }
}


static void Queue_prepend(Q self, QEntry new_entry)
{
    new_entry->next = self->head;
    new_entry->previous = NULL;

    switch(self->head == NULL) {
        case 1:
            self->tail = self->head = new_entry;
            break;
        case 0:
            self->head = self->head->previous = new_entry;
            break;
    }
}


static QEntry Queue_pop(Q self)
{
    if (Q_IS_EMPTY(self)) {
        return NULL;
    }
    QEntry entry = self->head;
    self->head = entry->next;
    entry->next = NULL;

    switch(self->head == NULL) {
        case 1:
            self->tail = self->head;
            break;
        case 0:
            self->head->previous = NULL;
            break;
    }
    return entry;
}
