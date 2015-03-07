#define _is_dead(a) ((CVObject *)a)->alive

int is_dead(PyObject *actor)
{
    int i;
    PyObject *a;
    PyGILState_STATE gstate;

    gstate = PyGILState_Ensure();
    a = PyWeakref_GET_OBJECT(actor);
    i = ((a == Py_None) || _is_dead(a));
    PyGILState_Release(gstate);
    return i;
}


void cv_control(SDL_UserEvent *event)
{
    CVCoroutine *coro = (CVCoroutine *)event->data1;
    
    if (!cv_check_coroutine(&coro->state)) {
    
