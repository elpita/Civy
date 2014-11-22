#define CV_EVENT_LOOP_START case 0: while(1) {
#define CV_EVENT_LOOP_END case 1:;}
#if defined(_WIN32) || defined(_WIN64)
#define sleep(dt) Sleep(1000*dt)
#endif


static SDL_Event _event;


static void cv_main_loop(void)
{
    switch(setjmp(main_jmp_buf)) {
        case -1:
            /* Call final function(s) */
            break;

        CV_EVENT_LOOP_START
            switch(SDL_PollEvent(&_event)) {
                case 0:
                    Py_BEGIN_ALLOW_THREADS
                    sleep(0.02);
                    Py_END_ALLOW_THREADS
                    break;
                default:
                    switch(_event.type) {
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
    }
}
