/* Coroutine Object */
#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 64
#endif /* CV_STACK_LENGTH */


struct _cvstack {
    CVContinuation cvstackptr;
    struct _cvcontinuation items[CV_STACK_LENGTH];
};


typedef struct _cvcoroutine *CVCoroutine;
struct _cvcoroutine {
    struct _cvstack stack;
    CVCoroutine parent;
    /* CVQ *actor_ptr; */
};


/* Actor Object */
typedef struct _cvqueueentry *CVQEntry;
struct _cvqueueentry {
    CVCoroutine routine;
    CVQEntry previous;
    CVQEntry next;
};

typedef struct _cvqueue *CVQ;
struct _cvqueue {
    CVQEntry head;
    CVQEntry tail;
};


typedef struct _cvactor *CVActor;
struct _cvactor {
    struct _cvqueue coroutines;
};
