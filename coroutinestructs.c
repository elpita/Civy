/* Continuation Object */
struct _cvargsstruct {
    PyObject *args;
    PyObject *kwds;
};


struct _cvcontext {
    int state;
    PyObject *passaround;
    void *vars;
};


typedef struct _cvcontinuation *CVContinuation;
struct _cvcontinuation {
    struct _cvcontext context;
    struct _cvargsstruct argsptr;
};


/* Coroutine Object */
#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 4
#endif /* CV_STACK_LENGTH */

struct _cvstack {
    struct _cvcontinuation items[CV_STACK_LENGTH];
    CVContinuation cvstackptr;
}

typedef struct _cvqueue *CVQ';
typedef struct _cvcoroutine *CVCoroutine;
struct _cvcoroutine {
    struct _cvstack stack;
    CVCoroutine parent;
    CVQ *actor_ptr;
};


/* Actor Object */
typedef struct _cvqueueentry *CVQEntry;
struct _cvqueueentry {
    CVCoroutine routine;
    CVQEntry previous;
    CVQEntry next;
};


struct _cvqueue {
    CVQEntry head;
    CVQEntry tail;
};


typedef struct _cvactor *CVActor;
struct _cvactor {
    CVQ coroutines;
};
