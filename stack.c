#include <stdio.h>
#include <stdlib.h>

#ifndef CV_STACK_LENGTH
#define CV_STACK_LENGTH 5
#endif /* CV_STACK_LENGTH */

typedef struct _cvstack *CVStack;
struct _cvstack {
    CVContinuation cvstackptr;
    struct _cvcontinuation items[CV_STACK_LENGTH];
}


CVStack CV_NewStack(void)
{
    CVStack s = (struct _cvstack *)PyMem_Malloc(sizeof(struct _cvstack));
    s->cvstackptr = s->items = NULL;
}


void CV_DeallocStack(CVStack s)
{
    PyObject *arg;

    for(arg = s->cvstackptr->argsptr; s->cvstackptr != NULL; arg = pop(s)->argsptr) {
        CV_DeallocArgs(arg);
    }
    PyMem_Free(s);
    *s = NULL;
}


void push(CVStack s, CVContinuation val)
{
    if (s->cvstackptr >= (s->items + STACK_SIZE)) {
       PyErr_SetString(PyExc_RuntimeError, "Continuation stack overflow.");
       /* jump back */
    }
    val->argsptr = NULL;
    *s->cvstackptr++ = *val;
}


CVContinuation pop(CVStack s)
{
    if (s->next == s->items) {
       return NULL;
    }
    return --s->next;
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
