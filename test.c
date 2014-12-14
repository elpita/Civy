#include "test.h"
#define CV_MAIN_LOOP_START switch(setjmp(to_main_loop)) { case 0: while(1) {
#define CV_MAIN_LOOP_END case 1: ; } break; case -1:
#define EXIT_CV break; }
static void (*cv_event_handlers[5]) (SDL_Event *);


static void cv_main_loop(void)
{
    CV_MAIN_LOOP_START

    switch(SDL_PollEvent(&main_event)) {
        case 0:
            Py_BEGIN_ALLOW_THREADS
            sleep(0.02);
            Py_END_ALLOW_THREADS
            break;
        default:
            switch(main_event.type) {
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
                default:
                    /* call some nonsense */
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
    int i, cv_m, cv_t, cv_a, cv_gc, cv_j, cv_ff, cv_fd;
    
    if PyObject_TypeCheck(self, &CVAppType) {
        PyErr_Format(PyExc_TypeError, "(%s) cannot be instantiated directly.", CVAppType.tp_name);
    }
    else if (args && (args != Py_None)) {
        PyErr_SetString(PyExc_TypeError, "init flags must be specified as keyword arguments.");
        return -1;
    }
    else {
        static char *kwargs[] = {"CV_MOUSE", "CV_TOUCH", "CV_AUDIO", "CV_GAME_CONTROLLER", "CV_JOYSTICK", "CV_FORCE_FEEDBACK", "CV_FILE_DROP", NULL};

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|iiiiiii", kwargs, &cv_m, &cv_t, &cv_a, &cv_gc, &cv_j, &cv_ff, &cv_fd)) {
            return -1;
        }
    }
    SDL_assert(!SDL_WasInit(SDL_INIT_EVERYTHING));
    mask = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    MAX_CV_INPUTS = i = 1 + (cv_m || cv_t) + cv_gc + cv_j + cv_fd;
    cv_event_handlers[i] = cv_handle_user_event;
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
    if cv_fd {
        SDL_EventState(SDL_DROPFILE, 1);
        cv_event_handlers[i] = cv_handle_dropfile_event;
        i--;
    }
    if cv_t {
        cv_event_handlers[i] = cv_handle_touch_event;
        i--;
    }
    if cv_m {
        cv_event_handlers[i] = cv_handle_mouse_event;
        i--;
    }

    tbd_global_app_pointer = self;
    return 0;
}


void cv_event_loop(SDL_Event *event)
{
    static int i;

    switch(setjmp(to_event_loop)) {
        case 0:
            for (i = 0; i <= MAX_CV_INPUTS; i++) {
                cv_event_handlers[i](event);
                case 1:;}
        case -1:
            i = 0;
            longjmp(to_main_loop, 1);
            break;
            
    }
}


cv_call_coroutine()
{
    if Q_IS_EMPTY(self->cvprocesses) {
        fail;
    }
    process = CVObject_pop_process(self);
    check_value = check_process(process);
    
    if (check_value == -1) {
        fail;
    }
    else if (check_value == 0) {
        deallocate(process);
        return;
    }
    
    if setjmp(process->jb) {
        deallocate(process);
        do_as_infinity();
        longjmp(to_main_loop, 1);
    }
    cv_call_continuation(process);
}


typedef _cvcontinuation *CVContinuation;
typedef _cvcontinuation_status *ConStatus;

struct _cvcontinuation_status {
    PyObject *handler;
    ConStatus parent;
};


struct _cvcontinuation {
    ConStatus status;
    _queue pipeline;
    jmp_buf to_con_loop;
    PyFrameObject *frame;
};


cv_call_continuation()
{
    switch(setjmp(c->to_con_loop)) { 
        case 0: 
            while(!whatever) {
                result = PyEval_CallObjectWithKeywords(self->fun, args, kwargs);
            }
        case 1:}
    longjmp(somthing, 1);
}


static int check_continuation(ConStatus c)
{
    if (c == NULL) {
        return 1;
    }
    else {
        PyObject *handler = PyWeakref_GetObject(c->handler);
        SDL_assert(handler != NULL);

        if (handler == Py_None) || (is_dead(handler)) {
            return 0;
        }
        return check_continuation(c->parent);
    }
}


static PyObject* something(argstruct *as)
{
    /* Py_EnterRecursiveCall() */
    PyObject *func = as->func;
    PyObject *args = as->args;
    PyObject *kwargs = as->kwargs;
    dereference(as);
    return PyEval_CallObjectWithKeywords(func, args, kwargs);
}
