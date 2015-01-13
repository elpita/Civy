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


static void cv_costack_push(CVCoStack s, CVContinuation c)
{
    if (s->s_ptr >= (s->items + STACK_SIZE)) {
       PyErr_SetString(PyExc_RuntimeError, "Overflow in coroutine stack.");
       /* jump back */
    }
    *s->s_ptr++ = *c;
}


static CVContinuation cv_costack_pop(CVCoStack s)
{
    if (s->s_ptr == s->items) {
       return NULL;
    }
    return --s->s_ptr;
}
