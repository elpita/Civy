/* Special thanks to Simon Howard, C Algorithms library */

typedef struct _QEntry {
    QEntry *previous;
    QEntry *next;
    } QEntry;


struct Q {
    QEntry *head;
    QEntry *next;
    };


Q* Q_new(void)
{
    Q *Q = (Q *)malloc(sizeof(Q));

    if Q == NULL
    {
        return NULL;
        }

    Q->head = Q->tail = NULL;
    return Q;
    }


void Q_dealloc(Q *q)
{
    while (!Q_is_empty(q))
    {
        q_pop(q);
        }

    free(q);
    }


void Q_push(Q *self)
{
    QEntry *new_entry = (QEntry *)malloc(sizeof(QEntry));

    new_entry->previous = self->tail;
    new_entry->next = NULL;

    if self->tail == NULL
    {
        self->head = self->tail = new_entry;
    else
        self->tail->next = self->tail = new_entry;
        }
    }


QEntry* Q_pop(Q *self)
{
    if self->head == NULL
    {
        return NULL;
        }

    QEntry *entry = self->head;
    self->head = entry->next;

    if self->head == NULL
    {
        self->tail = self->head;
    else
        self->head->previous = NULL;
        }

    return entry;
    }


int Q_is_empty(Q *q)
{
    return q->head == NULL;
    }
