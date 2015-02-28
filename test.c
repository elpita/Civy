#include <setjmp.h>

/* POSIX */
#if (defined(unix) || defined(__unix__) || defined(__unix)) || defined(__APPLE__)
#define cv_setjmp(jb) sigsetjmp(jb, 0)
#define cv_longjmp(jb, v) siglongjmp(jb, v)
static sigjmp_buf env[3];
#else /* Use normal setjmp */
#define cv_setjmp(jb) setjmp(jb)
#define cv_longjmp(jb, v) longjmp(jb, v)
static jmp_buf env[3];
#endif /* POSIX */

#define to_civy_end env[0]
#define to_main_loop env[1]
#define to_event_loop env[2]

#define CV_ENTER_MAIN_LOOP_HERE switch(cv_setjmp(to_main_loop)) { case 0: while(1) {
#define CV_EXIT_MAIN_LOOP_HERE case 1:;} break; }

#define CV_ENTER_EVENT_LOOP_HERE volatile int i; switch(cv_setjmp(to_event_loop)) { case 0:
#define CV_EXIT_EVENT_LOOP_HERE case 1:;} break; case -1: i = 0; cv_longjmp(to_main_loop, 1); break;}
static void (*cv_event_handlers[6]) (SDL_Event *);

#define PYOBJECT_NAME(ob) Py_TYPE(ob)->tp_name // For `PyErr_Format` handling


static void _cv_fail(void)
{/* Function to be called from the main interpreter thread
    in the event of an error, triggered in a different thread. */

    PyThreadState *tstate = PyThreadState_Get();
    PyThreadState_SetAsyncExc(tstate->thread_id, PyErr_Occurred());
}


static void cv_run(void)
{
    static PyFrameObject *main_frame;
    static int main_depth
    volatile int i;
    
    /* Begin CV by saving the current PyFrame, then restoring it when we're done. */

    i = cv_setjmp(to_civy_end);

    if (!i) {
        PyThreadState *tstate = PyThreadState_GET();

        main_frame = tstate->frame;
        main_depth = tstate->recursion_depth;
        Py_AtExit(SDL_Quit);
        cv_main_loop();
    }
    else {
        PyThreadState *tstate = PyThreadState_GET();

        tstate->frame = main_frame;
        tstate->recursion_depth = main_depth;

        if (i < 0) {
            PyErr_Print();
        }
    }
}


static void cv_main_loop(void)
{
    volatile SDL_Event main_event;

    CV_ENTER_MAIN_LOOP_HERE

    switch(SDL_PollEvent(&main_event)) {
        case 0:
            Py_BEGIN_ALLOW_THREADS
            SDL_Delay(0.02);
            Py_END_ALLOW_THREADS
            break;
        default:
            switch(main_event.type) {
                case CV_DISPATCHED_EVENT:
                    cv_dispatch_check(&main_event.user);
                    break;
                case SDL_WINDOWEVENT:
                    cv_handle_window_event(&main_event.window);
                    break;
                case SDL_KEYDOWN:
                    cv_handle_keyboard_event(&main_event.key);
                    break;
                case SDL_KEYUP:
                    cv_handle_keyboard_event(&main_event.key);
                    break;
                case SDL_TEXTINPUT:
                    cv_handle_textinput_event(&main_event.text);
                    break;
                case SDL_TEXTEDITING:
                    cv_handle_textedit_event(&main_event.edit);
                    break;

                /* Not (yet?) supported */
                case SDL_SYSWMEVENT:
                    break;
                case SDL_DOLLARGESTURE:
                    break;
                case SDL_DOLLARRECORD:
                    break;
                case SDL_RENDER_TARGETS_RESET:
                    break;
                case SDL_RENDER_DEVICE_RESET:
                    break;
                case SDL_FIRSTEVENT:
                    break;
                case SDL_LASTEVENT:
                    break;

                default:
                    cv_event_loop(&main_event);
                    break;
            }
            break;
    }

    CV_EXIT_MAIN_LOOP_HERE
}


static int cv_app_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    int i=0, cv_m=0, cv_tch=0;
    Uint32 mask = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS;

    if PyObject_TypeCheck(self, &CVAppType) {
        PyErr_Format(PyExc_NotImplementedError, "<%s> cannot be instantiated directly.", PYOBJECT_NAME(self));
        return -1;
    }
    else if (global_app_pointer) {
        PyErr_SetString(PyExc_RuntimeError, "Another application is in progress.");
        return -1;
    }
    else {
        static char *kwargs[] = {"CV_MOUSE", "CV_TOUCH", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii", kwargs, &cv_m, &cv_tch)) {
            return -1;
        }
    }
    SDL_assert(!SDL_WasInit(SDL_INIT_EVERYTHING));
    MAX_CV_INPUTS = cv_m + cv_tch;

    if (SDL_Init(mask) < 0) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }

    if cv_m {
        cv_event_handlers[i] = cv_handle_mouse_event;
        i++;
    }
    else {
        SDL_EventState(SDL_MOUSEMOTION, 0);
        SDL_EventState(SDL_MOUSEBUTTONDOWN, 0);
        SDL_EventState(SDL_MOUSEBUTTONUP, 0);
        SDL_EventState(SDL_MOUSEWHEEL, 0);
    }

    if cv_tch {
        cv_event_handlers[i] = cv_handle_touch_event;
        i++;
    }
    else {
        SDL_EventState(SDL_FINGERMOTION, 0);
        SDL_EventState(SDL_FINGERDOWN, 0);
        SDL_EventState(SDL_FINGERUP, 0);
        SDL_EventState(SDL_MULTIGESTURE, 0);
    }
    return 0;
}


void cv_event_loop(SDL_Event *event)
{
    CV_ENTER_EVENT_LOOP_HERE
        for (i = 0; i <= MAX_CV_INPUTS; i++) {
            cv_event_handlers[i](event);
    CV_EXIT_EVENT_LOOP_HERE

    //PyErr_Format(PyExc_RuntimeError, "Application received an unknown asynchronous event, %d.", event->type);
}


static void cv_handle_mouse_event(SDL_Event *event) {
    switch(event->type) {
        case SDL_MOUSEMOTION:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_MOUSEBUTTONDOWN:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_MOUSEBUTTONUP:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_MOUSEWHEEL:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        default:
            cv_longjmp(to_event_loop, 1);
    }
}


static void cv_handle_touch_event(SDL_Event *event) {
    if ((event->type == SDL_FINGERMOTION) || (event->type == SDL_FINGERDOWN) || (event->type == SDL_FINGERUP)) {
        /* call function */
        cv_longjmp(to_event_loop, -1);
    }
    else if (event->type == SDL_MULTIGESTURE) {
        /* call function */
        cv_longjmp(to_event_loop, -1);
    }
    else {
        cv_longjmp(to_event_loop, 1);
    }
}


static void cv_handle_dropfile_event(SDL_Event *event) {
    if (event->type == SDL_DROPFILE) {
        /* call function */
        cv_longjmp(to_event_loop, -1);
    }
    else {
        cv_longjmp(to_event_loop, 1);
    }
}


static void cv_handle_controller_event(SDL_Event *event) {
    switch(event->type) {
        case SDL_CONTROLLERAXISMOTION:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERBUTTONUP:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERDEVICEADDED:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERDEVICEREMAPPED:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        default:
            cv_longjmp(to_event_loop, 1);
    }
}


static void cv_handle_joystick_event(SDL_Event *event) {
    switch(event->type) {
        case SDL_JOYAXISMOTION:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_JOYBALLMOTION:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_JOYHATMOTION:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_JOYBUTTONDOWN:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_JOYBUTTONUP:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_JOYDEVICEADDED:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        case SDL_JOYDEVICEREMOVED:
            /* call function */
            cv_longjmp(to_event_loop, -1);
            break;
        default:
            cv_longjmp(to_event_loop, 1);
    }
}


void cv_handle_clipboard_event(SDL_Event *event) {
    if (event->type == SDL_CLIPBOARDUPDATE) {
        /* Looks like we just need to send a `True` */
        cv_longjmp(to_event_loop, -1);
    }
    else {
        cv_longjmp(to_event_loop, 1);
    }
}


void cv_handle_quit_event(SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        /* do stuff */
        cv_longjmp(to_main_loop, -1);
    }
    else {
        cv_longjmp(to_event_loop, 1);
    }
}


void cv_dispatch_check(SDL_UserEvent *event)
{
    PyObject *a = PyWeakref_GetObject(event->data);

    if is_dead(a) {
        Py_DECREF(event->data);
        return;
    }
    PyThreadState_GET()->recursion_depth = event->code;
    cv_continuation_check(cv_object_queue_pop(a->cvprocesses));
}


void cv_continuation_check(CVContinuation C)
{
    CVCoroState state = &C->state;

    if (!cv_check_continuation(state)) {
        cv_dealloc_coroutine(C);
        return;
    }
    else {
        CVCoStack stack = &C->stack;
        cv_user_loop(stack);

        if (state->parent != NULL) {
            schedule(state->parent);
            state->parent = NULL;
        }
        cv_dealloc_coroutine(C);
        cv_longjmp(to_main_loop, 1);
    }
}


static void cv_user_loop(CVCoStack stack)
{
    CVContinuation c;

    while (c = cv_stack_pop(stack)) {
        something = &c;
        c->cocall(c->coargs[0], c->coargs[1], c->coargs[2]);
    }
}


static int reschedule_current_continuation(CVCoroutine C, int line)
{
    struct _cvcontext *current_c = *context; //Should not be NULL
    current_c->state = line;

    if (cv_costack_push(C, (CVContinuation)current_c) < 0) {
        return -1;
    }
    return 0;
}


static cv_switch_routine(PyObject *actor, PyObject *args, PyObject *kwds, CVCallbackFunc *cocall, CVCleanupFunc *coclean, int from_python, int line)
{
    if (!CVEventDispatcher_Check(actor)) {
        PyErr_SetString("Only EventDispatchers may be scheduled blahblah"); //fix
        /* Jump */
    }
    else {
        CVCoroutine C = get_current_coroutine(actor);

        if (C == NULL) {
            /* Jump */
        }
        else {
            int depth = PyThreadState_GET()->recursion_depth;
            PyObject *weak_actor = PyWeakref_NewRef(actor, NULL);

            if (weak_actor == NULL) {
                cv_dealloc_coroutine(C); //hmmm...this might be a problem
                /* Jump */
            }
            else if (from_python && (async_schedule_rtp(C) < 0)) {
                Py_DECREF(weak_actor);
                cv_dealloc_coroutine(C);
                /* Jump */
            }
            else if (reschedule_current_continuation(C, line) < 0) {
                Py_DECREF(weak_actor);
                cv_dealloc_coroutine(C);
                /* Jump */
            }
            else {
                struct _cvcontinuation c = {{0, NULL}, cocall, coclean, {actor, args, kwds}};

                if (cv_costack_push(C, &c) < 0) {
                    cv_dealloc_coroutine(C);
                    Py_DECREF(weak_actor);
                    /* Jump */
                }
            }
            if (sdl_schedule(weak_actor, CV_DISPATCHED_EVENT, depth) < 0) {
                cv_dealloc_coroutine(C);
                Py_DECREF(weak_actor);
                /* Jump */
            }
            Py_DECREF(weak_actor);
        }
        if (cv_object_queue_push(actor->cvprocesses, C) < 0) {
             cv_dealloc_coroutine(C);
            /* Jump */
        }
        cv_longjmp(back, 1);
    }
}
