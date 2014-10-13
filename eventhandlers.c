#define _GET_PROPERTY_HANDLER(obj, str) PyDict_GetItemString( ((EventDispatcher)obj)->_properties, str )
#define GET_PROPERTY_HANDLER(obj, self) _GET_PROPERTY_HANDLER(obj, ((CVProperty)self)->name )


static int CVObject_schedule(CVObject self, PyObject *callback, PyObject *data)
{
    CVProcess process = CVProcess_new(self);

    if (process == NULL) {
        return -1;
    }
    PyGreenlet *g = PyGreenlet_New(callback, NULL);

    if (g == NULL) {
        CVProcess_dealloc(process);
        return -1;
    }
    else if (CVProcess_push_thread(process, g) < 0) {
        Py_DECREF(g);
        CVProcess_dealloc(process);
        return -1;
    }
    CVObject_push_process(self, process);
    return CV_join(self, data, DISPATCHED_EVENT);
}


struct _callbackhandler {
	CVObject super;
	PyObject *object;
	PyObject *observers;
	};
static PyTypeObject CallbackHandler_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "CallbackHandler",                                 /*tp_name*/
    sizeof(struct _callbackhandler),                          /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)CallbackHandler_dealloc,               /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash */
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,   /*tp_flags*/
    " ",                             /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,        /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    &CVObject_Type,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    CallbackHandler_init,                    /* tp_init */
    0,                                          /* tp_alloc */
    0,                               /* tp_new */
    };

 
static PyObject* CallbackHandler_init(CallbackHandler self, PyObject *args, PyObject *kwargs)
{
	self->object = NULL;
	self->observers = PyList_New(0);

	if (self->observers == NULL) {
		return -1;
	}
	return 0;
}


static void CallbackHandler_dealloc(PyObject *self)
{
	
}


struct _cvproperty {
	PyObject_HEAD
	const char *name;
	};
PyTypeObject CVProperty_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "property",                                 /* tp_name */
    sizeof(struct _cvproperty),                     /* tp_basicsize */
    0,                                          /* tp_itemsize */
    /* methods */
    property_dealloc,                           /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC |
        Py_TPFLAGS_BASETYPE,                    /* tp_flags */
    property_doc,                               /* tp_doc */
    property_traverse,                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    property_methods,                           /* tp_methods */
    property_members,                           /* tp_members */
    property_getsetlist,                        /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    property_descr_get,                         /* tp_descr_get */
    property_descr_set,                         /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    property_init,                              /* tp_init */
    PyType_GenericAlloc,                        /* tp_alloc */
    PyType_GenericNew,                          /* tp_new */
    PyObject_GC_Del,                            /* tp_free */
	};


static PyObject * CVProperty_descr_get(PyObject *self, PyObject *obj, PyObject *type)
{
    if (obj == NULL || obj == Py_None) {
        Py_INCREF(self);
        return self;
    }
    return GET_PROPERTY_HANDLER(obj, self)->object;
}


static int CVProperty_descr_set(PyObject *self, PyObject *obj, PyObject *value)
{
	if (value == NULL) {
		return -1; //?
	}
	PyObject *old_value, *callback;
	CallbackHandler handler = GET_PROPERTY_HANDLER(obj, self);
	old_value = handler->object;
	PyObject *args = PyTuple_Pack(3, obj, value, old_value);
	Py_DECREF(old_value);

	if (args == NULL) {
		return -1;
	}
	else if PyObject_HasAttr(obj, concat_string) {
		callback = _GET_PROPERTY_HANDLER(obj, concat_string); //fix this and figure out `callback` situation
		
		if (CVObject_schedule(obj, callback, args) < 0) {
			Py_DECREF(args);
			return -1;
		}
	}
	int i, len;
	PyObject *seq = PySequence_Fast(handler->observers, "expected a sequence of observers.");

	if (seq == NULL) {
		Py_DECREF(args);
		return -1;
	}
	for (i = 0; i < len; i++) {
		callback = PySequence_Fast_GET_ITEM(seq, i);

		if (callback == NULL) {
			Py_DECREF(seq);
			Py_DECREF(args);
			return -1;
		}
		/* Must decide how to grab the function in `callback` */
		/* g */
		else if (CVObject_schedule(obj, callback, args) < 0) {
			Py_DECREF(args);
			Py_DECREF(seq);
			return -1;
		}
	}
	Py_DECREF(args);
	Py_DECREF(seq);
	handler->object = value;
	Py_INCREF(value);
	return 0;
}


whatever
{
    const char *name;
    name = PyString_AsString(key);
    if (name[3] == "on_") && (PyString_Size(key) > 3) && (Py_TYPE(value) == PyFunction_Type) {
        PyObject *new_value = EventProperty(value);
        if (new_value == NULL) {
            return -1;
        }
        else if (PyDict_SetItem(self->cv, key, new_value) < 0) {
            Py_DECREF(new_value);
            return -1;
        }
        Py_DECREF(new_value);
    }

    else if (Py_TYPE(value) == Property) {
        if (PyDict_SetItem(self->cv, key, value) < 0) {
            return -1;
        }
    }
    


static PyObject *
EventDispatcherType_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    const char *name;
    Py_ssize_t pos = 0;
    PyObject *key, *value;

    while (PyDict_Next(kwds, &pos, &key, &value)) {
	name = PyString_AsString(key);
        if (name[3] == "on_") && (PyString_Size(key) > 3) && (Py_TYPE(value) == PyFunction_Type){
            PyObject *new_value = EventProperty(value);
            if (new_value == NULL)
                return -1;
            if (PyDict_SetItem(kwds, key, new_value) < 0) {
                Py_DECREF(new_value);
                return -1;
            }
            Py_DECREF(new_value);
        }
    }

    return type->tp_base->tp_new(type, args, kwds);
}


static PyTypeObject EventDispatcherType_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "eventdispatcher",                             /* tp_name */
    sizeof(PyType_Type),                           /* tp_basicsize */
    0,                                             /* tp_itemsize */
    0,                                             /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_compare */
    0,                                             /* tp_repr */
    0,                                             /* tp_as_number */
    0,                                             /* tp_as_sequence */
    0,                                             /* tp_as_mapping */
    0,                                             /* tp_hash */
    0,                                             /* tp_call */
    0,                                             /* tp_str */
    0,                                             /* tp_getattro */
    0,                                             /* tp_setattro */
    0,                                             /* tp_as_buffer */
    0,                                             /* tp_flags */
    0,                                             /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    0,                                             /* tp_iter */
    0,                                             /* tp_iternext */
    0,                                             /* tp_methods */
    0,                                             /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                                             /* tp_init */
    0,                                             /* tp_alloc */
    EventDispatcherType_new,                       /* tp_new */
};
