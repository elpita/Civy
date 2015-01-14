#include "civycoroutinestack.h"


struct _cvcostack {
    CVContinuation s_ptr;
    struct _cvcontinuation items[CV_STACK_LENGTH];
}


static void cv_init_costack(CVCoStack s)
{
    s->s_ptr = s->items;
}


static void cv_dealloc_costack(CVCoStack s)
{
    CVContinuation c;

    while ((c = cv_costack_pop(s)) != NULL) { //Not ANSI C, but i don't care.
        cv_dealloc_continuation(c);
    }
    //*s = NULL;
}


static int cv_costack_push(CVCoStack s, CVContinuation c)
{
    if (s->s_ptr >= (s->items + CV_STACK_LENGTH)) {
       PyErr_SetString(PyExc_RuntimeError, "Overflow in coroutine's stack.");
       return -1;
    }
    *s->s_ptr++ = *c;
    return 0;
}


static CVContinuation cv_costack_pop(CVCoStack s)
{
    if (s->s_ptr == s->items) {
       return NULL;
    }
    return --s->s_ptr;
}
