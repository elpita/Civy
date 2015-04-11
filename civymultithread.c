#define IF_RETURN_FROM_NESTED_DISPATCH break; default:
#define cv_save_continuation() (*context)->state = __LINE__

static void cv_exec(PyObject *actor_ptr, PyObject *name, PyObject *args)
{/* This is the special continuation for events called *from* Python  */

    PyObject *result;

    CV_ENTER_ROUTINE_HERE

    cv_save_continuation();
    _main_thread->frame = NULL;
    result = PyObject_CallMethodObjArgs(actor_ptr, name, args, NULL);

    IF_RETURNED_FROM_NESTED_DISPATCH
        result = cv_coresume();

    CV_EXIT_ROUTINE_HERE
    
    CV_CoReturn(result);
}


static void cv_periodic_exec(PyObject *ids, PyObject *key, PyObject *)
{/* This is the special continuation for events called *from* Python */
    PyObject *result = CV_CoResume();

    if ((result == Py_False) && (PyDict_Contains(ids, key)) {
        PyDict_DelItem(ids, key);
    }
    CV_CoReturn(result);
}


void cv_join(PyObject *args, PyObject *, PyObject *)
{/* This is a special continuation for returning *back* to Python */
    int throw_value;
    PyObject *result;

    {
        PyGILState_STATE gstate = PyGILState_Ensure();
        result = cv_coresume();
    
        /* Steal the reference to the frame */
        Py_INCREF(args);
        Py_XINCREF(result); //?

        /* If there was a problem, let the frame know */
        throw_value = PyErr_Occurred() ? 1 : 0;

        PyGILState_Release(gstate);
    }
    cv_kill_current();
    
    {
        PyFrameObject *_frame;
        {
            PyThreadState *tstate = _main_thread;

            _frame = (PyFrameObject *)args;
            tstate->frame = _frame;
        }
        *(_frame->f_stacktop++) = result;
        PyEval_RestoreThread(_main_thread);
        result = PyEval_EvalFrameEx(_frame, throw_value);
    }
    CV_CoReturn(result);
}


static int cv_push_event(CVCoroutine *coro, Uint32 event_type, int depth)
{
    SDL_Event event;

    SDL_zero(event);
    event.type = event_type;
    event.user.code = depth;
    event.user.data1 = (void *)coro;
    event.user.data2 = (void *)(*_cv_globals.passaround);

    if (SDL_PushEvent(&event) < 0) {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        PyGILState_Release(gstate);
        return 0;
    }
    return 1;
}


static int cv_dispatch(CVCoroutine *coro, PyObject *actor_ptr, PyObject *args, PyObject *kwargs, CVCallbackFunc cocall, CVCleanupFunc coclean, void *covars, int depth)
{
    CVCoStack *stack = &(coro->stack);
    CVContinuation contin = cv_create_continuation(actor_ptr, args, kwargs, cocall, coclean, covars);

    if (!cv_costack_push(stack, &contin)) {
        return 0;
    }
    else if (!cv_push_event(coro, CV_DISPATCHED_EVENT, depth)) {
        cv_dealloc_continuation(cv_costack_pop(stack));
        return 0;
    }
    return 1;
}


static CVCoroutine get_current_coroutine(PyObject *actor)
{
    CVCoroutine coro, parent=NULL;

    if (cv_current_coro != NULL) {
        parent = *cv_current_coro;

        if (cv_current_continuation != NULL) {
            CVContinuation contin = *cv_current_continuation;

            if (cv_costack_push(parent, contin) < 0) {
                return NULL;
            }
        }

        if (PyObject_RichCompareBool(actor, (PyObject *)(parent->state->actor_ptr), Py_EQ)) {
            return parent;
        }
    }
    coro = cv_create_coroutine(actor);

    if (coro == NULL) {
        return NULL;
    }
    coro->state->parent = parent;
    return coro;
}
