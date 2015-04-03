#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 128
#endif /* CV_STACK_LENGTH */


typedef struct _cvcostack {
    CVContinuation *s_ptr;
    _cvcontinuation items[CV_STACK_LENGTH];
} CVCoStack;


static void cv_init_costack(CVCoStack *s)
{
    s->s_ptr = s->items;
}


static int cv_costack_push(CVCoStack *s, CVContinuation *c)
{
    if (s->s_ptr >= (s->items + CV_STACK_LENGTH)) {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyErr_SetString(PyExc_RuntimeError, "Overflow in coroutine's stack.");
        PyGILState_Release(gstate);
        return 0;
    }
    *s->s_ptr++ = *c;
    return 1;
}


static CVContinuation* cv_costack_pop(CVCoStack *s)
{
    if (s->s_ptr == s->items) {
       return NULL;
    }
    return --s->s_ptr;
}


static void cv_dealloc_costack(CVCoStack *s)
{
    CVContinuation *c = cv_costack_pop(s);

    while (c != NULL) {
        cv_dealloc_continuation(c);
        c = cv_costack_pop(s);
    }
}
