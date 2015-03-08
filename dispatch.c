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
        PyGILState_STATE gstate = PyGILState_Ensure();
        CVCoroState *state = &(coro->state); 

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
