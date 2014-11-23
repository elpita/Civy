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
