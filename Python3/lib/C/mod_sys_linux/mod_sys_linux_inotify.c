/*
 * Copyright (c) 2020
 *      Christian CAMIER <christian.c@promethee.services>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "mod_sys_linux.h"
#include "mod_sys_linux_inotify.h"
#include "mod_sys_linux_inotify.d"

union freelist_un {
	union  freelist_un   *next;
	struct inotify_event  event;
	char                  data[4096];
};

union freelist_un *inotify_event_freelist = NULL;

static struct inotify_event *inotify_event_alloc(void);
static void                  inotify_event_free (struct inotify_event *);
static int                   mask_from_iterable(const char *, PyObject *, int *);
static PyObject             *mask_to_frozenset(int);

static PyObject *class_inotify_awa(object_inotify *self, PyObject *args, PyObject *kwds);
static PyObject *class_inotify_clo(object_inotify *self, PyObject *args, PyObject *kwds);
static void      class_inotify_del(object_inotify *self);
static PyObject *class_inotify_dwa(object_inotify *self, PyObject *args, PyObject *kwds);
static int       class_inotify_ini(object_inotify *self, PyObject *args, PyObject *kwds);
static PyObject *class_inotify_fno(object_inotify *self, PyObject *args, PyObject *kwds);
static PyObject *class_inotify_new(PyTypeObject   *type, PyObject *args, PyObject *kwds);
static PyObject *class_inotify_nxt(object_inotify *self, PyObject *args, PyObject *kwds);
static PyObject *class_inotify_rep(object_inotify *self);

static void      class_inotifyevent_del(object_inotifyevent *self);
static PyObject *class_inotifyevent_gat(object_inotifyevent *self, int attrcode);
static PyObject *class_inotifyevent_rep(object_inotifyevent *self);
//static object_inotifyevent *object_inotifyevent_new(void);

/*
 * The Inotify Object definition
 */
static PyMemberDef class_inotify_members[] = {
	{ "filefd", T_UINT,      offsetof(object_inotify, fileno), READONLY, "File descriptor of the Inotify" },
	{ NULL, 0, 0, 0, NULL }
};

static PyMethodDef class_inotify_methods[] = {
	{ "addWatch",     (PyCFunction)class_inotify_awa,  METH_VARARGS | METH_KEYWORDS,  addwatch_doc },
	{ "close",        (PyCFunction)class_inotify_clo,  METH_NOARGS,                   close_doc    },
	{ "delWatch",     (PyCFunction)class_inotify_dwa,  METH_VARARGS | METH_KEYWORDS,  delwatch_doc },
	{ "fileno",       (PyCFunction)class_inotify_fno,  METH_NOARGS,                   fileno_doc   },
	{ "next",         (PyCFunction)class_inotify_nxt,  METH_VARARGS | METH_KEYWORDS,  next_doc     },
	{ NULL, NULL, 0, NULL },
};

static PyTypeObject class_inotify = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".Inotify",		/* tp_name */
	sizeof(object_inotify),			/* tp_basicsize */
	0,					/* tp_itemsize */
	(destructor)class_inotify_del,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_reserved */
	(reprfunc)class_inotify_rep,		/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	0,					/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	(const char *)class_inotify_doc,	/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	class_inotify_methods,			/* tp_methods */
	class_inotify_members,			/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	(initproc)class_inotify_ini,		/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	class_inotify_new,			/* tp_new */
	PyObject_Del,				/* tp_free */
};

/*
 * The Inotify Event Object definition
 */
static PyGetSetDef class_inotifyevent_attrs[] = {
	{ "cookie",  (getter)class_inotifyevent_gat, NULL, "Unique cookie associating related events ", (void *)ATTR_IEV_COOKIE },
	{ "len",     (getter)class_inotifyevent_gat, NULL, "Size of name attribute",                    (void *)ATTR_IEV_LEN    },
	{ "mask",    (getter)class_inotifyevent_gat, NULL, "Mask describing event",                     (void *)ATTR_IEV_MASK   },
	{ "name",    (getter)class_inotifyevent_gat, NULL, "Optionnal Name",                            (void *)ATTR_IEV_NAME   },
	{ "watchid", (getter)class_inotifyevent_gat, NULL, "Watch Id",                                  (void *)ATTR_IEV_WD     },
	{ NULL, NULL, NULL, NULL, NULL }										         
};															         

static PyTypeObject class_inotifyevent = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".Inotifyevent",		/* tp_name */
	sizeof(object_inotifyevent),		/* tp_basicsize */
	0,					/* tp_itemsize */
	(destructor)class_inotifyevent_del,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_reserved */
	(reprfunc)class_inotifyevent_rep,	/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	0,					/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	(const char *)class_inotifyevent_doc,	/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0,					/* tp_methods */
	0,					/* tp_members */
	class_inotifyevent_attrs,		/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	//(initproc)class_inotifyevent_ini,		/* tp_init */
	0,					/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	0,					/* tp_new */
	PyObject_Del,				/* tp_free */
};

static struct inotify_event *inotify_event_alloc(void)
{
	union freelist_un *nevnt = inotify_event_freelist;

	debug(">> inotify_event_alloc\n");
	if(NULL == nevnt)
	{
		if(NULL == (nevnt = (union freelist_un *)malloc(sizeof(union freelist_un))))
		{
			PyErr_Format(PyExc_MemoryError, "Cannot allocate InotifyEvent object");
			debug("!! inotify_event_alloc\n");
			return NULL;
		}
		nevnt->next = NULL;
	}
	inotify_event_freelist = nevnt->next;
	debug("<< inotify_event_alloc\n");
	return (struct inotify_event *)nevnt;
}

static void inotify_event_free(struct inotify_event *event)
{
	union freelist_un *fevnt = (union freelist_un *)event;
	debug(">> inotify_event_free\n");
	fevnt->next = inotify_event_freelist;
	inotify_event_freelist = fevnt;
	debug("<< inotify_event_free\n");
	return;
}

static int mask_from_iterable(const char *entity, PyObject *aiterable, int *amask)
{
	long      smask = 0;
	PyObject *sset  = NULL;
	debug(">> mask_from_iterable\n");
	if(NULL == (sset = PySet_New(aiterable)))
	{
		PyErr_Format(PyExc_TypeError, "%s is not an iterable", entity);
		goto fail;
	}
	while(0 < PySet_GET_SIZE(sset))
	{
		PyObject *elem;
		long      snum;
		if(NULL == (elem = PySet_Pop(sset)))
		{
			char errstr[80];
			snprintf(errstr, sizeof(errstr), "mask_from_iterable for %s: Internal error", entity);
			PyErr_SetString(PyExc_RuntimeError, errstr);
			goto fail;
		}
		if(!PyLong_Check(elem))
		{
			char errstr[80];
			snprintf(errstr, sizeof(errstr), "%s number is not an integer", entity);
			PyErr_SetString(PyExc_ValueError, errstr);
			Py_DECREF(elem);
			goto fail;
		}
		snum = PyLong_AsLong(elem);
		Py_DECREF(elem);
		smask |= snum;
	}
	*amask = (int)smask;
	debug("<< mask_from_iterable\n");
	return 1;
fail:
	if(NULL != sset)
	{
		while(0 <  PySet_GET_SIZE(sset)) Py_DECREF(PySet_Pop(sset));
		Py_DECREF(sset);
	}
	debug("!! mask_from_iterable\n");
	return 0;
}

static PyObject *mask_to_frozenset(int mask)
{
#define MOD_SL_INT_NBITS	(8 * sizeof(int))

	size_t    cbit = 0;
	size_t    rmsk = 0;
	PyObject *ofst = NULL;
	PyObject *oset = NULL;

	debug(">> mask_to_frozenset");
	if(NULL == (oset = PySet_New(NULL)))
		goto fail;
	for(cbit = 0, rmsk = 1; cbit < MOD_SL_INT_NBITS; rmsk <<= 1, cbit += 1)
	{
		if(mask & (int)rmsk)
			PySet_Add(oset, PyLong_FromLong((long)cbit));
	}
	if(NULL == (ofst = PyFrozenSet_New(oset)))
		goto fail;
	Py_DECREF(oset);
	debug("<< mask_to_frozenset");
	return ofst;

fail:
	if(NULL != oset) Py_DECREF(oset);
	if(NULL != ofst) Py_DECREF(ofst);
	debug("!! mask_to_frozenset");
	return NULL;

#undef MOD_SL_INT_NBITS
}

static PyObject *class_inotify_awa(object_inotify *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "pathname", "events", NULL };

	PyObject *amask = NULL;
	int       imask = 0;
	char     *apath = NULL;
	int       wd    = 0;

	debug(">> class_inotify_awa\n");
	if(!PyArg_ParseTupleAndKeywords(args, kwds, "sO", (char **)keywords, &apath, &amask)) goto fail;
	if(!mask_from_iterable("events", amask, &imask))                                      goto fail;
	if(-1 == (wd = inotify_add_watch(self->fileno, apath, imask)))
 	{
		PyErr_SetFromErrno(PyExc_SystemError);
		goto fail;
	}
	debug("<< class_inotify_awa\n");
	return Py_BuildValue("K", wd);

fail:
	if(NULL != amask)
	{
		while(0 <  PySet_GET_SIZE(amask)) Py_DECREF(PySet_Pop(amask));
		Py_DECREF(amask);
	}
	debug("!! class_inotify_awa\n");
	Py_RETURN_NONE;
}

static PyObject *class_inotify_clo(object_inotify *self, PyObject *args, PyObject *kwds)
{
	if(-1 != self->fileno)
	{
		close(self->fileno);
		self->fileno = -1;
	}
	Py_RETURN_NONE;
}

static void class_inotify_del(object_inotify *self)
{
	PyObject_Del(self);
	return;
}

static PyObject *class_inotify_dwa(object_inotify *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "watch", NULL };
	int                watchid;
	if(PyArg_ParseTupleAndKeywords(args, kwds, "i", (char **)keywords, &watchid))
		if(-1 == inotify_rm_watch(self->fileno, watchid))
			PyErr_SetFromErrno(PyExc_SystemError);
	Py_RETURN_NONE;
}

static int  class_inotify_ini(object_inotify *self, PyObject *args, PyObject *kwds)
{
	(void)self;
	(void)args;
	(void)kwds;
	return 0;
}
static PyObject *class_inotify_fno(object_inotify *self, PyObject *args, PyObject *kwds)
{
	if(-1 == self->fileno)
		Py_RETURN_NONE;
	return PyLong_FromLong(self->fileno);
}

static PyObject *class_inotify_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	static char *keywords[] = { "path", "flags", NULL };

	object_inotify *new    = NULL;
	PyObject       *aflags = NULL;
	int             iflags =  0;
	int             fileno = -1;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "|O", (char **)keywords, &aflags)) return NULL;
	if(NULL != aflags && -1 == mask_from_iterable("flags", aflags, &iflags))       return NULL;
	if(-1 == (fileno = inotify_init1(iflags)))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return NULL;
	}
	if(NULL != (new = (object_inotify *)type->tp_alloc(type, 0)))
		new->fileno = fileno;
	return (PyObject *)new;
}

static PyObject *class_inotify_nxt(object_inotify *self, PyObject *args, PyObject *kwds)
{
#define MOD_SL_NAME_OFFSET	(size_t)(((struct inotify_event *)0)->name)
#define MOD_SL_NAME_MAX         (sizeof(union freelist_un) - MOD_SL_NAME_OFFSET)
#define MOD_SL_NAME_LAST        MOD_SL_NAME_MAX - 1

	object_inotifyevent  *retv = NULL;
	struct inotify_event *buff = NULL;
	ssize_t               nbrd =    0;

	if(NULL == (buff = inotify_event_alloc()))
		return NULL;
	
	switch(nbrd = read(self->fileno, (void *)buff, sizeof(struct inotify_event)))
	{
	case -1:
		if(EAGAIN != errno || EWOULDBLOCK != errno)
		{
			inotify_event_free(buff);
			PyErr_SetFromErrno(PyExc_SystemError);
			goto fail;
		}
		/* Fall into */
	case  0:
		inotify_event_free(buff);
		retv = (object_inotifyevent *)Py_None;
		Py_INCREF(Py_None);
		break;
	default:
		if     (nbrd <= (ssize_t)MOD_SL_NAME_OFFSET)        buff->name[0]                         = '\0';
		else if(nbrd <  (ssize_t)sizeof(union freelist_un)) buff->name[nbrd - MOD_SL_NAME_OFFSET] = '\0';
		else                                                buff->name[MOD_SL_NAME_LAST]          = '\0';
	}
	if(NULL == (retv = (object_inotifyevent *)class_inotifyevent.tp_alloc(&class_inotifyevent, 0)))
		goto fail;
	retv->event = buff;
	return (PyObject *)retv;

fail:
	if(buff) inotify_event_free(buff);
	if(retv) Py_DECREF(retv);
	return NULL;

#undef MOD_SL_NAME_LAST
#undef MOD_SL_NAME_MAX
#undef MOD_SL_NAME_OFFSET
}

static PyObject *class_inotify_rep(object_inotify *self)
{
	return PyUnicode_FromFormat("<%s object at %p (fileno = %d)>", Py_TYPE(self)->tp_name, self, self->fileno);
}

static void class_inotifyevent_del(object_inotifyevent *self)
{
	inotify_event_free(self->event);
	PyObject_Del(self);
	return;
}

static PyObject *class_inotifyevent_gat(object_inotifyevent *self, int attrcode)
{
	switch(attrcode)
	{
	case ATTR_IEV_COOKIE: return PyLong_FromLong((long)self->event->cookie);
	case ATTR_IEV_LEN:    return PyLong_FromLong((long)self->event->len);
	case ATTR_IEV_MASK:   return mask_to_frozenset( self->event->mask);
	case ATTR_IEV_NAME:   return PyUnicode_FromFormat("%s", self->event->name);
	case ATTR_IEV_WD:     return PyLong_FromLong((long)self->event->wd);
	default:              break;
	}
	PyErr_Format(PyExc_AttributeError, "%s: unknown attribute", Py_TYPE(self)->tp_name);
	return NULL;
}

static PyObject *class_inotifyevent_rep(object_inotifyevent *self)
{
	//PyObject *retv = NULL;
	//PyObject *mstr = NULL;
	//PyObject *mset = NULL;

	//mset = mask_to_frozenset(self->event->mask);
	//mstr = PyObject_Repr(mset);

	return PyUnicode_FromFormat("<%s object at %p>", Py_TYPE(self)->tp_name, self);
}

#if 0
static object_inotifyevent *object_inotifyevent_new(void)
{
}
#endif

/*
 * Module initialization
 */
static int mod_sys_linux_inotify_init(PyObject *module)
{
	MODULE_SL_ADD_CONSTANT(IN_ACCESS);
	MODULE_SL_ADD_CONSTANT(IN_ALL_EVENTS);
	MODULE_SL_ADD_CONSTANT(IN_ATTRIB);
	MODULE_SL_ADD_CONSTANT(IN_CLOEXEC);
	MODULE_SL_ADD_CONSTANT(IN_CLOSE);
	MODULE_SL_ADD_CONSTANT(IN_CLOSE_NOWRITE);
	MODULE_SL_ADD_CONSTANT(IN_CLOSE_WRITE);
	MODULE_SL_ADD_CONSTANT(IN_CREATE);
	MODULE_SL_ADD_CONSTANT(IN_DELETE);
	MODULE_SL_ADD_CONSTANT(IN_DELETE_SELF);
	MODULE_SL_ADD_CONSTANT(IN_DONT_FOLLOW);
	MODULE_SL_ADD_CONSTANT(IN_EXCL_UNLINK);
	MODULE_SL_ADD_CONSTANT(IN_IGNORED);
	MODULE_SL_ADD_CONSTANT(IN_ISDIR);
	MODULE_SL_ADD_CONSTANT(IN_MASK_ADD);
	MODULE_SL_ADD_CONSTANT(IN_MODIFY);
	MODULE_SL_ADD_CONSTANT(IN_MOVE);
	MODULE_SL_ADD_CONSTANT(IN_MOVED_FROM);
	MODULE_SL_ADD_CONSTANT(IN_MOVED_TO);
	MODULE_SL_ADD_CONSTANT(IN_MOVE_SELF);
	MODULE_SL_ADD_CONSTANT(IN_NONBLOCK);
	MODULE_SL_ADD_CONSTANT(IN_ONESHOT);
	MODULE_SL_ADD_CONSTANT(IN_ONLYDIR);
	MODULE_SL_ADD_CONSTANT(IN_OPEN);
	MODULE_SL_ADD_CONSTANT(IN_Q_OVERFLOW);
	MODULE_SL_ADD_CONSTANT(IN_UNMOUNT);

	Py_TYPE(&class_inotify) = &PyType_Type;
	if(-1 == PyType_Ready(&class_inotifyevent))
		return 0;

	Py_INCREF((PyObject *)&class_inotify);
	Py_INCREF((PyObject *)&class_inotifyevent);
	if(0 != PyModule_AddObject(module, "Inotify",      (PyObject *)&class_inotify     )) return 0;
	if(0 != PyModule_AddObject(module, "InotifyEvent", (PyObject *)&class_inotifyevent)) return 0;

	return 1;
}

//MODULE_SL_ADD_FUNCTION(sigpending,  METH_NOARGS);
MODULE_SL_ADD_INITIALIZER(mod_sys_linux_inotify_init)


