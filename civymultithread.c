#define IF_RETURN_FROM_NESTED_DISPATCH break; default:
#define cv_save_continuation() (*context)->state = __LINE__


/*static void reset_arguments(PyObject *args)
{ Switch out the weak ref with the real actor (assume it's still alive)
    PyObject *obj, *actor;

    obj = PyTuple_GET_ITEM(args, 0);
    actor = PyWeakref_GET_OBJECT(obj);
    Py_INCREF(actor);
    PyTuple_SET_ITEM(args, 0, actor);
    //Py_DECREF(obj);
}*/


/* static void cv_exec(PyObject *func, PyObject *args, PyObject *kwds)
{ This is the special continuation for events called *from* Python 
    PyObject *result;

    CV_ENTER_ROUTINE_HERE

    cv_save_continuation();
    PyThreadState_GET()->frame = NULL;
    reset_arguments(args);
    result = PyObject_Call(func, args, kwds); // cheating

    IF_RETURNED_FROM_NESTED_DISPATCH
        result = CV_CoResume();

    CV_EXIT_ROUTINE_HERE

    CV_CoReturn(result);
} */


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


static int cv_spawn(CVCoroutine coroutine, PyObject *func, PyObject *args, PyObject *kwds)
{/* Used for creating a new coroutine called from python */
    _cvcontinuation cfp = {{0, NULL}, cv_exec, NULL, {NULL, NULL, NULL}}; //Also called in SDL Threads
    PyObject *arguments[3] = {func, args, kwds};

    cfp.coargs = arguments;
    
    if (cv_costack_push(coroutine, &cfp) < 0) {
        return -1;
    }
    return 0;
}


static int cv_push_event(CVCoroutine *coro, Uint32 event_type, int depth)
{
    SDL_Event event;

    SDL_zero(event);
    event.type = event_type;
    event.user.code = depth;
    event.user.data1 = (void *)coro;
    event.user.data2 = (void *)cv_coreturn(); // Keep an eye on this...

    if (SDL_PushEvent(&event) < 0) {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        PyGILState_Release(gstate);
        return 0;
    }
    return 1;
}


static void cv_dispatch(CVCoroutine *coro, PyObject *actor_ptr, PyObject *args, PyObject *kwargs, CVCallbackFunc cocall, CVCleanupFunc coclean, void *covars, int depth)
{
    if (from_python) {
        CVContinuation contin;
        CVCoStack *stack = &(coro->stack);
        PyObject *_frame = (PyObject *)(_main_thread->frame);

        contin = cv_create_continuation(_frame, NULL, NULL, cv_join, NULL, NULL);

        /* Keep the frame's reference count at 1 */
        {
            PyGILState_STATE gstate = PyGILState_Ensure();
            Py_DECREF(_frame);
            PyGILState_Release(gstate);
        }

        if (!cv_costack_push(stack, &contin)) {
            FAIL();
        }
    }{
        CVCoStack *stack = &(coro->stack);
        CVContinuation contin = cv_create_continuation(actor_ptr, args, kwargs, cv_exec, NULL, NULL);

        if (!cv_costack_push(stack, &contin)) {
            FAIL();
        }
    }{
        int depth = _main_thread->recursion_depth;

        if (!cv_push_event(coro, CV_DISPATCHED_EVENT, depth)) {
            FAIL();
        }
    }
}


static int cv_wait(CVCoroutine coroutine)
{/* Used for creating the coroutine that returns to python */
    static _cvcontinuation rtp = {{0, NULL}, cv_join, NULL, {NULL, NULL, NULL}};
    PyObject *frame;
    int depth;

    frame = (PyObject *)PyEval_GetFrame();
    rtp.coargs[0] = frame;

    if (cv_costack_push(coroutine, &rtp) < 0) {
        return -1;
    }
    return 0;
}


static int cv_fork(CVCoroutine coro, PyObject *func, PyObject *args, PyObject *kwds)
{/* Used for creating a synchronous python event */

    if (cv_wait(coro) < 0) {
        return -1;
    }
    else if (cv_spawn(coro, func, args, kwds) < 0) {
        return -1;
    }
    else if (cv_push_event(PyTuple_GET_ITEM(args, 0), CV_DISPATCHED_EVENT, PyThreadState_GET()->recursion_depth) < 0) {
        return -1;
    }
    return 0;
}


static int dispatch_sync_event(CVObject actor, PyObject *a, PyObject *b, PyObject *c)
{
    CVCoroutine coro = cv_create_coroutine((PyObject *)actor);

    if (coro == NULL) {
        return -1;
    }
    else if (cv_spawn_sync_event(coro, a, b, c) < 0) {
        //cv_dealloc_coroutine(coro);
        return -1;
    }
    else if (cv_object_queue_push(actor->cvprocesses, coro) < 0) {
        //cv_dealloc_coroutine(coro);
        return -1;
    }
    return 0;
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
