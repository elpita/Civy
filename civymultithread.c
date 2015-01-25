#define IF_RETURN_FROM_NESTED_DISPATCH break; default:
#define cv_save_continuation() (*context)->state = __LINE__


static void reset_arguments(PyObject *args)
{/* Switch out the weak ref with the real actor (assume it's still alive) */
    PyObject *obj, *actor;

    obj = PyTuple_GET_ITEM(args, 0);
    actor = PyWeakref_GET_OBJECT(obj);
    Py_INCREF(actor);
    PyTuple_SET_ITEM(args, 0, actor);
    //Py_DECREF(obj);
}


static void cv_push_event(PyObject *target, Uint32 event_type, int depth)
{
    SDL_Event event;

    SDL_zero(event);
    event.type = event_type;
    event.user.code = depth;
    event.user.data1 = target;
    Py_INCREF(target);

    if (SDL_PushEvent(&event) < 0) {
        Py_DECREF(target);
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }
    return 0;
}


static void cv_exec(PyObject *func, PyObject *args, PyObject *kwds)
{/* This is the special continuation for events called *from* Python */
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
}


void cv_join(PyObject *args, PyObject *, PyObject *)
{/* This is a special continuation for returning *back* to Python */
    PyObject *result = cv_coresume();
    PyThreadState *ts = PyThreadState_GET();

    /* steal the reference to the frame */
    Py_INCREF(args);
    Py_INCREF(result); //?
    cv_kill_current();

    //ts->recursion_depth = CV_GetRoutineVars();
    ts->frame = (PyFrameObject *)args;
    *(ts->frame->f_stacktop++) = result;
    result = PyEval_EvalFrame(ts->frame);
    CV_CoReturn(result);
}


static int cv_spawn(CVCoroutine coroutine, PyObject *func, PyObject *args, PyObject *kwds)
{/* Used for creating a new coroutine called from python */
    static struct _cvcontinuation cfp = {{0, NULL}, cv_exec, NULL, {NULL, NULL, NULL}};

    cfp.coargs[0] = func;
    cfp.coargs[1] = args;
    cfp.coargs[2] = kwds;

    if (cv_costack_push(coroutine, &cfp) < 0) {
        return -1;
    }
    return 0;
}


static int cv_wait(CVCoroutine coroutine)
{/* Used for creating the coroutine that returns to python */
    static struct _cvcontinuation rtp = {{0, NULL}, cv_join, NULL, {NULL, NULL, NULL}};
    PyThreadState *tstate;
    PyObject *frame;
    int depth;

    tstate = PyThreadState_GET();
    frame = (PyObject *)(tstate->frame);
    depth = tstate->recursion_depth;
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
