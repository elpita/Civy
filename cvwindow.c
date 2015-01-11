#include <string.h>

typedef struct _cvwindow *CVWindow;
struct _cvwindow {
    struct _cvobject super;
    SDL_Window *cwindow;
    SDL_GLContext gl_context;
}


static PyObject* CVWindow_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    char *name;
    SDL_Window *window;
    SDL_GLContext gl_context;
    int x=SDL_WINDOWPOS_UNDEFINED, y=SDL_WINDOWPOS_UNDEFINED, w=800, h=600;
    CVWindow self = (struct _cvwindow *)CVObject_new(type, args, kwargs);

    if (self == NULL) {
        return NULL;
    }
    else {
        char buffer[33];
        static char *kwargs[] = {"name", "size", "pos", NULL};

        sprintf(buffer, "Window %d", 1 + PyDict_Size(app_chldrn));
        name = &buffer;

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s(i,i)(i,i)", kwargs, &name, &x, &y, &w, &h)) {
            Py_DECREF(self);
            return NULL;
        }
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    window = SDL_CreateWindow(name, x, y, w, h, SDL_WINDOW_OPENGL);

    if (window = NULL) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        Py_DECREF(self);
        return NULL;
    }
    gl_context = SDL_GL_CreateContext(window);

    if (gl_context = NULL) {
        PyErr_SetString(PyExc_RuntimeError, SDL_GetError());
        SDL_DestroyWindow(window);
        Py_DECREF(self);
        return NULL;
    }
    self->window = window;
    self->gl_context = gl_context;
    return (PyObject *)self;
}


static int CVWindow_init(CVWindow self, PyObject *args, PyObject *kwargs)
{
    PyObject *win_id = Py_BuildValue("I", SDL_GetWindowID(self->window));

    if (win_id == NULL) {
        return -1;
    }
    if (PyDict_SetItem(app_chldrn, self, win_id) < 0) {
        PyDECREF(win_id);
        return -1;
    }
    Py_DECREF(win_id);
    /* Set name, size, and position */
    return 0;
}


static void CVWindow_dealloc(CVWindow self)
{
    SDL_GL_DeleteContext(self->gl_context);
    SDL_DestroyWindow(self->cwindow);
    CVObject_dealloc((CVObject)self);
}
