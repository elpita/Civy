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
            PyObject *ret = event->data2;
            Py_XDECREF(ret);
            PyGILState_Release(gstate);
            cv_dealloc_coroutine(coro);
            return;
        }
        else {
            PyObject *current = PyWeakref_GET_OBJECT(state->actor_ptr);
            Py_INCREF(current);
            PyGILState_Release(gstate);
            _cv_globals.current_actor = current;
        }
        _cv_globals.current_coroutine = coro;
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
        case -1: {
            PyObject *current;
            PyGILState_STATE gstate;

            cv_dealloc_coroutine(coro);
            current = _cv_globals.current_actor;
            gstate = PyGILState_Ensure();
            Py_DECREF(global_actor);
            PyGILState_Release(gstate);
            cv_longjmp(to_main_loop, -1);
        }
        case 1: {
            PyObject *current;
            PyGILState_STATE gstate;

            current = _cv_globals.current_actor;
            gstate = PyGILState_Ensure();
            Py_DECREF(global_actor);
            PyGILState_Release(gstate);
            cv_longjmp(to_main_loop, 1);
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
