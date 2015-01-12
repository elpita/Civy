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


static PyObject* CVWindow_on_pos(CVWindow self, PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_size(CVWindow self, PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_visible(CVWindow self, PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_minimized(CVWindow self)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_restored(CVWindow self)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_maximized(CVWindow self)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_mouse_focus(CVWindow self, PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_keyboard_focus(CVWindow self, PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject* CVWindow_on_close(CVWindow self)
{
    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef CVWindow_methods[] = {
    {"on_pos", (PyCFunction)CVWindow_on_pos, METH_VARARGS,
     "Callback for when the Window changes position"
    },
    {"on_size", (PyCFunction)CVWindow_on_size, METH_VARARGS,
     "Callback for when the Window changes size"
    },//Don't forget SDL_WINDOWEVENT_SIZE_CHANGED!
    {"on_visible", (PyCFunction)CVWindow_on_visible, METH_VARARGS,
     "Callback for when the Window's visibility changes"
    },
    {"on_minimized", (PyCFunction)CVWindow_on_minimized, METH_NOARGS,
     "Callback for when the Window is minimized"
    },
    {"on_restored", (PyCFunction)CVWindow_on_restored, METH_NOARGS,
     "Callback for when the Window is restored to normal size and position"
    },
    {"on_maximized", (PyCFunction)CVWindow_on_maximized, METH_NOARGS,
     "Callback when the Window is maximized"
    },
    {"on_mouse_focus", (PyCFunction)CVWindow_on_mouse_focus, METH_VARARGS,
     "Callback when the Wwindow has gained, or lost, the mouse's focus"
    },
    {"on_keyboard_focus", (PyCFunction)CVWindow_on_keyboard_focus, METH_VARARGS,
     "Callback when the Wwindow has gained, or lost, the keyboard's focus"
    },
    {"on_close", (PyCFunction)CVWindow_on_close, METH_NOARGS,
     "Callback when the Window closes"
    },
    {NULL}  /* Sentinel */
};


static void CVWindow_dealloc(CVWindow self)
{
    SDL_GL_DeleteContext(self->gl_context);
    SDL_DestroyWindow(self->cwindow);
    CVObject_dealloc((CVObject)self);
}


static PyTypeObject CVWindowType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /* ob_size */
    "civy.Window",                              /* tp_name */
    sizeof(struct _cvwindow),                   /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)CVWindow_dealloc,               /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Window objects",                           /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    CVWindow_methods,                           /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    &CVObject_Type,                             /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)CVWindow_init,                    /* tp_init */
    0,                                          /* tp_alloc */
    CVWindow_new,                               /* tp_new */
};
