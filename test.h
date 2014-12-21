#ifndef CIVYMAIN
#define CIVYMAIN

static jmp_buf env[3];
#define to_civy_end env[0]
#define to_main_loop env[1]
#define to_event_loop env[2]
#endif /* CIVYMAIN */
