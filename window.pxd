cdef extern from "civy.h":
    cdef extern class civy.Window [object CVWindow, type CVWindow_Type]:
        pass


cdef extern class civy.window.ClickableWindow(civy.Window):
    pass

cdef extern class civy.window.TouchableWindow(civy.Window):
    pass
