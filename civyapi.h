#ifndef CIVYAPI
#define CIVYAPI

typedef struct _cvframecontext *CVFrameContext;
static CVFrameContext *context;

#define CV_GetRoutineVars() (*context)->passaround
#define CV_BEGIN_ROUTINE switch((*context)->state) { case 0:
#define CV_SwitchRoutine(r, a, b, c) \
    do { \
        sleep_the(context); \
        set_context(a, b, c); \
        (*context)->state = __LINE__; \
        longjmp(back, 1); \
        case __LINE__: \
            r = (*context)->passaround; \
        } while (0)

#define CV_END_ROUTINE }
#define CV_ReturnRoutine(r) \
    (*context)->passaround = r; \
    longjmp(back, 1);

#endif /* CIVYAPI */
