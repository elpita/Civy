#include <stdio.h>
#include <stdlib.h>

#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 5
#endif /* CV_STACK_LENGTH */

typedef struct _cvstack *CVStack;
struct _cvstack {
    CVContinuation s_ptr;
    struct _cvcontinuation items[CV_STACK_LENGTH];
}


CVStack CV_InitStack(CVStack s)
{
    s->s_ptr = s->items;
}


void CV_DeallocStack(CVStack s)
{
    PyObject *arg;
    CVContinuation c;
    
    while ((c = pop(s)) != NULL) { //Not ANSI C, but i don't care.
        CV_DeallocArgs(c->argsptr);
    }
    //*s = NULL;
}


void push(CVStack s, CVContinuation val)
{
    if (s->s_ptr >= (s->items + STACK_SIZE)) {
       PyErr_SetString(PyExc_RuntimeError, "Overflow in coroutine stack.");
       /* jump back */
    }
    val->argsptr = NULL;
    *s->s_ptr++ = *val;
}


CVContinuation pop(CVStack s)
{
    if (s->s_ptr == s->items) {
       return NULL;
    }
    return --s->s_ptr;
}


int main(void)
{
    Stack s;
    init(&s);

    srand(time(NULL));

    for (int i = 0; i < STACK_SIZE; i++)
        push(&s, (double)rand());

    printf("-----------\n");

    for (int i = 0; i < STACK_SIZE; i++)
        pop(&s);

    return 0;
}     
