static enum { STARTED, ACTIVE, DEAD } state;


static int pop()
{
    _process = _pop(_current->pipeline);
    if (_process == NULL) {
        return 0;
    }
    return 1;
}


static void process_routine(void)
{
    switch(setjmp(_process->jmp)) {
        case 1:
            roll(); //nullify currrent thread
            push_routine();
            break;
        default:
            switch(_process->state) {
                case STARTED:
                    _process->state = ACTIVE;
                    return;
                    while(pop_routine()) {
                        unroll(); //reinstate saved thread
                        result = PyEval_CallObjectWithKeywords(_routine->func, args, kwargs);
                        case ACTIVE:;
                    }
            }
    }
    longjmp(_main->jmp, 1);
}


static void actor_routine(void)
{
    switch(setjmp(_current->jmp)) {
        case 1:
            roll();
            break;
        default:
            switch(_current->state) {
                case STARTED:
                    _current->state = ACTIVE;
                    return;
                    while(pop()) {
                        unroll();
                        process_routine();
                        case ACTIVE:;
                    }
            }
    }
    longjmp(_main->jmp, 1);
}
