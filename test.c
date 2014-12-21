#include "test.h"
#define CV_MAIN_LOOP_START switch(setjmp(to_main_loop)) { case 0: while(1) {
#define CV_MAIN_LOOP_END case 1: ; } break; case -1:
#define CV_EVENT_LOOP_START static int i; switch(setjmp(to_event_loop)) { case 0:
#define CV_EVENT_LOOP_END case 1:;} break; case -1: i = 0; longjmp(to_main_loop, 1); break;}
#define EXIT_CV break; }
static void (*cv_event_handlers[6]) (SDL_Event *);


static void cv_main_loop(void)
{
    volatile SDL_Event main_event;

    CV_MAIN_LOOP_START

    switch(SDL_PollEvent(&main_event)) {
        case 0:
            Py_BEGIN_ALLOW_THREADS
            sleep(0.02);
            Py_END_ALLOW_THREADS
            break;
        default:
            switch(main_event.type) {
                case CV_DISPATCHED_EVENT:
                    cv_dispatch_check(main_event.user.data1, main_event.user.data2);
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

    CV_MAIN_LOOP_END

    /* Call final function(s) */
    EXIT_CV
}


static int cv_app_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    Uint32 mask;
    int i, cv_m, cv_tch, cv_a, cv_gc, cv_j, cv_ff, cv_df, cv_c;
    
    if PyObject_TypeCheck(self, &CVAppType) {
        PyErr_Format(PyExc_NotImplementedError, "<%s> cannot be instantiated directly.", CVAppType.tp_name);
        return -1;
    }
    else if (args && (args != Py_None)) {
        PyErr_SetString(PyExc_TypeError, "init flags must be specified as keyword arguments.");
        return -1;
    }
    else {
        static char *kwargs[] = {"CV_MOUSE", "CV_TOUCH", "CV_AUDIO", "CV_GAME_CONTROLLER", "CV_JOYSTICK", "CV_FORCE_FEEDBACK", "CV_DROP_FILE", "CV_CLIPBOARD", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|iiiiiiii", kwargs, &cv_m, &cv_tch, &cv_a, &cv_gc, &cv_j, &cv_ff, &cv_df, &cv_c)) {
            return -1;
        }
    }
    SDL_assert(!SDL_WasInit(SDL_INIT_EVERYTHING));
    mask = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    MAX_CV_INPUTS = i = 1 + (cv_m || cv_tch) + cv_gc + cv_j + cv_df + cv_c;
    cv_event_handlers[i] = cv_handle_quit_event;
    i--;

    if cv_ff {
        mask |= SDL_INIT_HAPTIC;
    }
    if cv_a {
        mask |= SDL_INIT_AUDIO;
    }
    if cv_j {
        mask |= SDL_INIT_JOYSTICK;
        cv_event_handlers[i] = cv_handle_joystick_event;
        i--;
    }
    if cv_gc {
        mask |= SDL_INIT_GAMECONTROLLER;
        cv_event_handlers[i] = cv_handle_controller_event;
        i--;
    }
    if (SDL_Init(mask) < 0) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }
    if cv_c {
        SDL_EventState(SDL_CLIPBOARDUPDATE, 1);
        cv_event_handlers[i] = cv_handle_clipboard_event;
        i--;
    }
    if cv_df {
        SDL_EventState(SDL_DROPFILE, 1);
        cv_event_handlers[i] = cv_handle_dropfile_event;
        i--;
    }

    if cv_tch {
        cv_event_handlers[i] = cv_handle_touch_event;
        i--;
    }
    else {
        SDL_EventState(SDL_FINGERMOTION, 0);
        SDL_EventState(SDL_FINGERDOWN, 0);
        SDL_EventState(SDL_FINGERUP, 0);
        SDL_EventState(SDL_MULTIGESTURE, 0);
    }

    if cv_m {
        cv_event_handlers[i] = cv_handle_mouse_event;
        i--;
    }
    else {
        SDL_EventState(SDL_MOUSEMOTION, 0);
        SDL_EventState(SDL_MOUSEBUTTONDOWN, 0);
        SDL_EventState(SDL_MOUSEBUTTONUP, 0);
        SDL_EventState(SDL_MOUSEWHEEL, 0);
    }

    tbd_global_app_pointer = self;
    return 0;
}


void cv_event_loop(SDL_Event *event)
{
    CV_EVENT_LOOP_START
        for (i = 0; i <= MAX_CV_INPUTS; i++) {
            cv_event_handlers[i](event);
    CV_EVENT_LOOP_END

    //PyErr_Format(PyExc_RuntimeError, "Application received an unknown asynchronous event, %d.", event->type);
}


void cv_handle_dispatched_event(data1, data2)
{
    if (data1 == NULL) {
        cleanup(data2);
        longjmp(to_main_loop, 1);
    }
    else {
        cv_continue(event->data1, event->data2);
    }
}


static void cv_handle_mouse_event(SDL_Event *event) {
    switch(event->type) {
        case SDL_MOUSEMOTION:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_MOUSEBUTTONDOWN:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_MOUSEBUTTONUP:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_MOUSEWHEEL:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        default:
            longjmp(to_event_loop, 1);
    }
}


static void cv_handle_touch_event(SDL_Event *event) {
    if ((event->type == SDL_FINGERMOTION) || (event->type == SDL_FINGERDOWN) || (event->type == SDL_FINGERUP)) {
        /* call function */
        longjmp(to_event_loop, -1);
    }
    else if (event->type == SDL_MULTIGESTURE) {
        /* call function */
        longjmp(to_event_loop, -1);
    }
    else {
        longjmp(to_event_loop, 1);
    }
}


static void cv_handle_dropfile_event(SDL_Event *event) {
    if (event->type == SDL_DROPFILE) {
        /* call function */
        longjmp(to_event_loop, -1);
    }
    else {
        longjmp(to_event_loop, 1);
    }
}


static void cv_handle_controller_event(SDL_Event *event) {
    switch(event->type) {
        case SDL_CONTROLLERAXISMOTION:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERBUTTONUP:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERDEVICEADDED:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_CONTROLLERDEVICEREMAPPED:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        default:
            longjmp(to_event_loop, 1);
    }
}


static void cv_handle_joystick_event(SDL_Event *event) {
    switch(event->type) {
        case SDL_JOYAXISMOTION:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_JOYBALLMOTION:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_JOYHATMOTION:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_JOYBUTTONDOWN:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_JOYBUTTONUP:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_JOYDEVICEADDED:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        case SDL_JOYDEVICEREMOVED:
            /* call function */
            longjmp(to_event_loop, -1);
            break;
        default:
            longjmp(to_event_loop, 1);
    }
}


void cv_handle_clipboard_event(SDL_Event *event) {
    if (event->type == SDL_CLIPBOARDUPDATE) {
        /* Looks like we just need to send a `True` */
        longjmp(to_event_loop, -1);
    }
    else {
        longjmp(to_event_loop, 1);
    }
}


void cv_handle_quit_event(SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        /* do stuff */
        longjmp(to_main_loop, -1);
    }
    else {
        longjmp(to_event_loop, 1);
    }
}


static int check_continuation(ConStatus c)
{
    if (c == NULL) {
        return 1;
    }
    else if (*c == NULL) {
        return 0;
    }
    return check_continuation(c->parent);
}


void cv_dispatch_check(a, b)
{
    if (is_empty(a)) {
        /* cleanup references */
        return;
    }
    cv_continuation = pop(a);

    if (!continuation_check(cv_continuation)) {
        /* cleanup */
        return;
    }
    callsomething(cv_continuation, b);
}


void callsometing(continuation, b)
{
    volatile PyThreadState *ts;

    switch(setjmp(to_continuation_loop)) {
        case -1:
            ts = PyThreadState_GET();
            continuation->frame = ts->frame;
            continuation->recursion_depth = ts->recursion_depth;
            break;
        case 0:
            ts = PyThreadState_GET();
            ts->frame = continuation->frame;
            ts->recursion_depth = continuation->recursion_depth;

            for (c = something_pop(); c != NULL; c = something_pop()) {
                c->call(&b);
                case 1:;
            }
            /* Go back to python */
            Py_INCREF(b);
            *(ts->frame->f_stacktop++) = b;
            b = PyEval_EvalFrame(ts->frame);
            Py_XDECREF(b);
            deallocate(continuation);
            break;
    }
    longjmp(to_main_loop, 1);
}


void func(co *something, PyObject **result)
{
    switch(something->state) {
        case ALIVE:
            /* set jmp_buf here */
            /* Call callback in python */
            *result = PyEval_CallObjectWithKeywords(something->func, something->args, something->kwds);
        case DEAD:
            Py_XDECREF(something->func);
            Py_XDECREF(something->args);
            Py_XDECREF(something->kwds);
            deallocate(something);
        case YIELD:
            longjmp(to_continuation_loop, -1);
            break;
    }
}
