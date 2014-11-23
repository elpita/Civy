#include "test.h"
#define CV_EVENT_LOOP_START switch(setjmp(to_main_loop)) { case 0: while(1) {
#define CV_EVENT_LOOP_END case 1: ; } break; case -1:
#define EXIT_CV break; }


static void cv_main_loop(void)
{
    CV_EVENT_LOOP_START

    switch(SDL_PollEvent(&main_event)) {
        case 0:
            Py_BEGIN_ALLOW_THREADS
            sleep(0.02);
            Py_END_ALLOW_THREADS
            break;
        default:
            switch(main_event.type) {
                case DISPATCHED_EVENT:
                    /* call some nonsense */
                    break;
                default:
                    /* call some other nonsense */
                    break;
            }
            break;
    }

    CV_EVENT_LOOP_END

    /* Call final function(s) */
    EXIT_CV
}


static int cv_app_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    Uint32 mask;
    char *string;
    Py_ssize_t i, len;
    PyObject *obj, *seq, *str_item;
    char *cv_a = "CV_AUDIO", *cv_gc = "CV_GAME_CONTROLLER", *cv_j = "CV_JOYSTICK", *cv_ff = "CV_FORCE_FEEDBACK";

    SDL_assert(!SDL_WasInit(SDL_INIT_EVERYTHING));
    mask = SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS;

    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return -1;
    }
    seq = PySequence_Fast(obj, "expected a sequence of strings.");
    
    if (seq == NULL) {
        return -1;
    }
    len = PySequence_Size(seq);

    if (len  < 0) {
        Py_DECREF(seq);
        return -1;
    }
    for (i = 0; i < len; i++) {
        str_item = PySequence_Fast_GET_ITEM(seq, i);
        string = PyString_AsString(str_object);

        if (string == NULL) {
            Py_DECREF(seq);
            return -1;
        }
        else if (strcmp(string, cv_a) == 0) {
            mask |= SDL_INIT_AUDIO;
        }
        else if (strcmp(string, cv_gc) == 0) {
            mask |= SDL_INIT_GAMECONTROLLER;
        }
        else if (strcmp(string, cv_j) == 0) {
            mask |= SDL_INIT_JOYSTICK;
        }
        else if (strcmp(string, cv_ff) == 0) {
            mask |= SDL_INIT_HAPTIC;
        }
    }
    if (SDL_Init(mask) < 0) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        return -1;
    }
    tbd_global_app_pointer = self;
    Py_DECREF(seq);
    return 0;
}
