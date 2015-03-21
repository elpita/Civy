#define _is_dead(a) ((CVObject *)a)->alive

static int is_dead(PyObject *actor)
{
    PyObject *a = PyWeakref_GET_OBJECT(actor);
    return ((a == Py_None) || _is_dead(a));
}


static void cv_control(SDL_UserEvent *event)
{
    volatile CVCoroutine *coro = (CVCoroutine *)(event->data1);

    {
        PyGILState_STATE gstate;
        CVCoroState *state = &(coro->state); 

        gstate = PyGILState_Ensure();

        if (!cv_check_coroutine(state)) {
            Py_XDECREF((PyObject *)(event->data2)); //Fix me
            cv_dealloc_coroutine(coro);
            PyGILState_Release(gstate);
            return;
        }
        global_actor = PyWeakref_GET_OBJECT(state->actor_ptr);
        Py_INCREF(global_actor);
        PyGILState_Release(gstate);
    }

    switch(cv_setjmp(to_whatever)) {
        case 0: {
            CVStack *stack = &(coro->stack)
            coroutine_call(stack);
        }
        case -1:
            cv_dealloc_coroutine(coro);
            /* Cleanup */
        case 1: {
            PyGILState_STATE gstate = PyGILState_Ensure();
            Py_DECREF(global_actor);
            PyGILState_Release(gstate);
        }
    }
    /* Cleanup */
}


static void coroutine_call(CVCoStack *stack)
{
    volatile CVContinuation *c = cv_costack_pop(stack);

    global_continuation = &c;
    switch(cv_setjmp(to_this)) {
        case 0:
            while (c != NULL) {
                CVCallbackFunc cocall = c->cocall;
                PyObject *args[3]; = c->coargs;

                cocall(args[0], args[1], args[2]);
                /* assert zero */
                case 1: c = cv_costack_pop(stack);}
    }
}   