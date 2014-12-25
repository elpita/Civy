#ifndef CIVYAPI
#define CIVYAPI

typedef struct _cvframecontext *CVFrameContext;
static CVFrameContext *context;

#define CV_GetRoutineVars() (void *)((*context)->vars)
#define CV_SetRoutineVars(_vars) (*context)->vars = (void *)_vars
#define CV_ENTER_ROUTINE switch((*context)->state) { case 0:
#define CV_SwitchRoutine(r, a, b, c) \
    do { \
        sleep_the(context); \
        set_context(a, b, c); \
        (*context)->state = __LINE__; \
        longjmp(back, 1); \
        case __LINE__: \
            r = (*context)->passaround; \
    } while(0)

#define CV_EXIT_ROUTINE }
#define CV_CoReturn(r) \
    do { \
        (*context)->passaround = (PyObject *)r; \
        longjmp(back, 1); \
    } while(0)

#endif /* CIVYAPI */
