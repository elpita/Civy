/* Special thanks to Simon Howard, C Algorithms library */

#include <stdlib.h>
#include "q.h"

/* Structures */
struct QueueEntry {
    QEntry previous;
    QEntry next;
    };

struct Queue {
    QueueEntry head;
    QueueEntry next;
    };


/* Functions */
static Q Queue_new(void)
{
    Q q = (Queue *)malloc(sizeof(Queue));

    if (q == NULL) {
        return NULL;
    }
    q->head = q->tail = NULL;
    return q;
}


static void Queue_push(Q self, QEntry new_entry)
{
    new_entry->previous = self->tail;
    new_entry->next = NULL;

    switch(self->tail == NULL) {
        case 1:
            self->head = self->tail = new_entry;
            break;
        case 0:
            self->tail->next = self->tail = new_entry;
            break;
    }
}


static void Queue_prepend(Q self, QEntry new_entry)
{
    new_entry->next = self->head;
    new_entry->previous = NULL;

    switch(self->head == NULL) {
        case 1:
            self->head = self->tail = new_entry;
            break;
        case 0:
            self->head->previous = self->head = new_entry;
            break;
    }
}


static QEntry Queue_pop(Q self)
{
    if (Q_IS_EMPTY(self)) {
        return NULL;
    }
    QueueEntry *entry = self->head;
    self->head = entry->next;

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



#define DOT_QUEUE_NEW 0
#define DOT_QUEUE_DEALLOC 1
#define DOT_QUEUE_PUSH 2
#define DOT_QUEUE_PREPEND 3
#define DOT_QUEUE_POP 4

#define q_DOT_Queue_new (*(Queue *) IMPORT_q[DOT_QUEUE_NEW])
