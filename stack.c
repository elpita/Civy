#include <stdio.h>
#include <stdlib.h>

typedef struct _cvstack *CVStack;
struct _cvstack {
    CVContinuation cvstackptr;
    struct _cvcontinuation items[CV_STACK_LENGTH];
}


CVStack CV_NewStack(CVStack s)
{
    s->next = s->items;
}


void push(CVStack s, CVContinuation val)
{
    if (s->next >= s->items + STACK_SIZE)
       fprintf(stderr, "stack overflow\n");
    else
    {
        *s->next++ = val;
        printf("push %g\n", val);
    }
}


void pop(CVStack s)
{
    if (s->next == s->items)
       fprintf(stderr, "stack underflow\n");
    else
        printf("pop %g\n", *--s->next);
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
