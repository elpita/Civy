/*
 * picoro - minimal coroutines for C.
 * Written by Tony Finch <dot@dotat.at>
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>

#include "picoro.h"


static struct coro {
	struct coro *next;
	jmp_buf state;
} first, *running = &first, *idle;


int resumable(coro c) {
	return(c != NULL && c->next == NULL);
}


static void push(coro *list, coro c) {
	c->next = *list;
	*list = c;
}


static coro pop(coro *list) {
	coro c = *list;
	*list = c->next;
	c->next = NULL;
	return(c);
}


static void *pass(coro me, void *arg) {
	static void *saved;
	saved = arg;
	if(!setjmp(me->state))
		longjmp(running->state, 1);
	return(saved);
}


void *resume(coro c, void *arg) {
	assert(resumable(c));
	push(&running, c);
	return(pass(c->next, arg));
}


void *yield(void *arg) {
	return(pass(pop(&running), arg));
}


void coroutine_start(void), coroutine_main(void*);


coro coroutine(void *fun(void *arg)) {
	if(idle == NULL && !setjmp(running->state))
		coroutine_start();
	return(resume(pop(&idle), fun));
}


void coroutine_main(void *ret) {
	void *(*fun)(void *arg);
	struct coro me;
	push(&idle, &me);
	fun = pass(&me, ret);
	if(!setjmp(running->state))
		coroutine_start();
	for(;;) {
		ret = fun(yield(&me));
		push(&idle, pop(&running));
		fun = pass(&me, ret);
	}
}


void coroutine_start(void) {
	char stack[16 * 1024];
	coroutine_main(stack);
}

