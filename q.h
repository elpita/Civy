#ifndef q_included
#define q_included

typedef struct QueueEntry *QEntry;
typedef struct Queue *Q;


static Q Queue_new(void);
static void Queue_dealloc(Q self);
static void Queue_push(Q self, QEntry new_entry);
static void Queue_prepend(Q self, QEntry new_entry);
static QEntry Queue_pop(Q self);
#define Q_IS_EMPTY(q) (q->head == NULL)


static void **IMPORT_q = NULL;

#define DOT_QUEUE_NEW           0
#define DOT_QUEUE_DEALLOC       1
#define DOT_QUEUE_PUSH          2
#define DOT_QUEUE_PREPEND       3
#define DOT_QUEUE_POP           4

#define q_DOT_Queue_new         (*(Queue *)IMPORT_q[DOT_QUEUE_NEW])
#define q_DOT_Queue_dealloc     (*(void *)IMPORT_q[DOT_QUEUE_DEALLOC])
#define q_DOT_Queue_push        (*(void *)IMPORT_q[DOT_QUEUE_PUSH])
#define q_DOT_Queue_prepend     (*(void *)IMPORT_q[DOT_QUEUE_PREPEND])
#define q_DOT_Queue_pop         (*(QueueEntry *)IMPORT_q[DOT_QUEUE_POP])
#endif
