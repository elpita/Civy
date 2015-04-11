#ifndef CIVYAPI
#define CIVYAPI

typedef struct _cvframecontext *CVFrameContext;
static CVFrameContext *context;
void (*func)(PyObject* actor, PyObject* args, PyObject *kwds);

#define CV_GetRoutineVars() (void *)((*context)->vars)
#define CV_SetRoutineVars(largs) (*context)->vars = (void *)largs
#define CV_ENTER_ROUTINE_HERE switch((*context)->state) { case 0:
#define cv_coresume(r) \
    do { \
        PyObject *ret = (*_cv_globals.passaround); \
        (*_cv_globals.passaround) = NULL; \
        Py_XDECREF(ret); \
        r = ret; \
    } while(0)

#define CV_SwitchRoutine(r, a, b, c) \
    do { \
        sleep_the(context); \
        set_context(a, b, c); \
        (*context)->state = __LINE__; \
        longjmp(back, 1); \
        case __LINE__: \
            cv_coresume(r); \
    } while(0)

#define cv_kill_current() \
    do { \
        cv_dealloc_args( ((CVContinuation)(*context))->argsptr ); \
        context = NULL; \
    } while(0)
#define CV_CoReturn(r) \
    do { \
        PyObject *ret = (PyObject *)r; \
        (*_cv_globals.passaround) = ret \
        Py_XINCREF(ret); \
        _cv_globals._main_thread = PyEval_SaveThread(); \
        cv_longjmp(to_continuation, 1); \
    } while(0)

#define CV_EXIT_ROUTINE_HERE break;}

#endif /* CIVYAPI */
