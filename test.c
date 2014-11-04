static enum { STARTED, ACTIVE, YIELDED, DEAD } state;


static void actor_routine(void)
{
    switch(_current->state) {
        case STARTED:
            switch(setjmp(_current->jmp)) {
                case 1:
                    unroll();
                    get_sp(something);
                    longjmp(_main->jmp);
            }
            _current->state = YIELDED;
            return;
            while(1) {
                _current->state = ACTIVE;
                case ACTIVE:
                    _process = pop(_current->pipeline);
                    roll(_process->local_stack);
                    call(_process);
                case YIELDED:;
            }
    }
}
