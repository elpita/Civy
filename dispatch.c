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
        CVCoroState *state = (CVCoroState *)coro; 

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
        global_coroutine = coro;
    }

    switch(cv_setjmp(to_coroutine)) {
        case 0: {
            CVStack *stack = &(coro->stack)
            coroutine_call(stack);
        }{
            CVCoroutine *parent = ((CVCoroState *)coro)->parent;

            if (parent != NULL) {
                int depth = _main_thread->recursion_depth;
                PyObject *result = (PyObject *)event->data2;

                if (cv_push_event(parent, result, CV_DISPATCHED_EVENT, depth) < 0) {
                    /* cleanup */
                    cv_longjmp(to_main_loop, -1);
                }
                ((CVCoroState *)coro)->parent = NULL;
            }
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
    switch(cv_setjmp(to_continuation)) {
        case 0:
            while (c != NULL) {
                CVCallbackFunc cocall = c->cocall;
                PyObject *args[3]; = c->coargs;

                cocall(args[0], args[1], args[2]);
                /* assert zero */
                case 1: c = cv_costack_pop(stack);}
    }{
        PyGILState_STATE gstate;
        PyObject *error_occurred;

        gstate = PyGILState_Ensure();
        error_occurred = PyErr_Occurred();
        PyGILState_Release(gstate);

        if (error_occurred) {
            cv_dealloc_coroutine(global_coroutine);
            cv_longjmp(to_coroutine, -1);
        }
    }
    //cv_longjmp(to_whatever, 1);
}
