typedef struct _QEntry {
    QEntry *previous;
    QEntry *next;
    } QEntry;


struct Q {
    QEntry *head;
    QEntry *next;
    };


Q* Q_new(void);
void Q_dealloc(Q *q);
int Q_is_empty(Q *q);
void Q_push(Q *self, QEntry *new_entry);
QEntry* Q_pop(Q *self);
