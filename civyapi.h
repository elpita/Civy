#ifndef CIVYAPI
#define CIVYAPI

typedef struct _cvframecontext *CVFrameContext;
static CVFrameContext *context;
void (*func)(PyObject* actor, PyObject* args, PyObject *kwds);

#define CV_GetRoutineVars() (void *)((*context)->vars)
#define CV_SetRoutineVars(largs) (*context)->vars = (void *)largs
#define CV_ENTER_ROUTINE_HERE switch((*context)->state) { case 0:
#define cv_coresume() \
    do {
        PyObject *r = (*passaround); \
        Py_XDECREF(r); \
        return r; \
    } while(0) //This is WRONG

#define CV_SwitchRoutine(r, a, b, c) \
    do { \
        sleep_the(context); \
        set_context(a, b, c); \
        (*context)->state = __LINE__; \
        longjmp(back, 1); \
        case __LINE__: \
            r = cv_coresume(); \
    } while(0)

#define cv_kill_current() \
    do { \
        cv_dealloc_args( ((CVContinuation)(*context))->argsptr ); \
        context = NULL; \
    } while(0)
#define CV_CoReturn(r) \
    do { \
        (*passaround) = (PyObject *)r; \
        Py_XINCREF(r); \
        cv_kill_current(); \
        return; \
    } while(0)

#define CV_EXIT_ROUTINE_HERE break;}

#endif /* CIVYAPI */
