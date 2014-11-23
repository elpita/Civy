#ifndef CIVYMAIN
#define CIVYMAIN

#if defined(_WIN32) || defined(_WIN64)
#define sleep(dt) Sleep(1000*dt)
#endif

static volatile SDL_Event main_event;
static jmp_buf to_main_loop;
#endif
