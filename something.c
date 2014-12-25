typedef struct _cvcontinuation *CVContinuation;
struct _cvcontinuation {
    int state;
    PyObject *passaround;
    void *vars;
};


#define STACK_LENGTH 4


typedef struct _cvcoroutine *CVCoroutine;
struct _cvcoroutine {
    struct _cvcontinuation stack[STACK_LENGTH];
    CVCoroutine parent;
};
