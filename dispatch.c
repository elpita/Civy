#define _is_dead(a) ((CVObject *)a)->alive

int is_dead(PyObject *actor)
{
    PyObject *a = PyWeakref_GET_OBJECT(actor);
    return ((a == Py_None) || _is_dead(a));
}


void cv_control(SDL_UserEvent *event)
{
    CVCoroutine *coro = (CVCoroutine *)(event->data1);

    {
        PyGILState_STATE gstate = PyGILState_Ensure();
        CVCoroState *state = &(coro->state); 

        if (!cv_check_coroutine(state)) {
            Py_XDECREF((PyObject *)(event->data2)); //Fix me
            cv_dealloc_coroutine(coro);
            PyGILState_Release(gstate);
            return
        }
        global_actor = PyWeakref_GET_OBJECT(state->actor_ptr);
        Py_INCREF(global_actor);
        PyGILState_Release(gstate);
    }

    {
        CVStack *stack = &(coro->stack)
        
        coroutine_call(stack);
    }
    /* Cleanup */
}
