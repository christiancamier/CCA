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

#include <sys/signalfd.h>

#include <signal.h>
#include <string.h>

#include "mod_sys_linux.h"
#include "mod_sys_linux_signalfd.h"
#include "mod_sys_linux_signalfd.d"

/*
 * Defined functions
 */

/* Local defined functions */
static PyObject *class_signalfd_clo(object_signalfd *self, PyObject *args, PyObject *kwds);
static void      class_signalfd_del(object_signalfd *self);
static PyObject *class_signalfd_fno(object_signalfd *self, PyObject *args, PyObject *kwds);
static int       class_signalfd_ini(object_signalfd *self, PyObject *args, PyObject *kwds);
static PyObject *class_signalfd_new(PyTypeObject    *type, PyObject *args, PyObject *kwds);
static PyObject *class_signalfd_nsi(object_signalfd *self, PyObject *args, PyObject *kwds);
static PyObject *class_signalfd_ssi(object_signalfd *self, PyObject *args, PyObject *kwds);

static void      class_signalinfo_del(object_signalinfo *self);
static PyObject *class_signalinfo_gat(object_signalinfo *self, int attrcode);
static PyObject *class_signalinfo_rep(object_signalinfo *self);
static object_signalinfo *object_signalinfo_new(void);

static PyObject *frozenset_from_sigmask(sigset_t *sigmask);
static int       sigmask_from_iterable(PyObject *asigset, sigset_t *sigmask);

static PyObject *meth_sigpending (PyObject *module, PyObject *args, PyObject *kwds);
static PyObject *meth_sigprocmask(PyObject *module, PyObject *args, PyObject *kwds);
static PyObject *meth_sigsuspend (PyObject *module, PyObject *args, PyObject *kwds);

union freelist_un {
        union freelist_un       *next;
        struct signalfd_siginfo  signfo;
};

static union freelist_un *signalfd_siginfo_freelist = NULL;

/* SIGNALFD Object */
static PyMemberDef class_signalfd_members[] = {
	{ "filefd", T_UINT,      offsetof(object_signalfd, fileno), READONLY, "File descriptor of the SignalFd" },
	{ "sigset", T_OBJECT_EX, offsetof(object_signalfd, sigset), READONLY, "Expected signals"                },
	{ NULL, 0, 0, 0, NULL }
};

static PyMethodDef class_signalfd_methods[] = {
	{ "close",        (PyCFunction)class_signalfd_clo,  METH_NOARGS,                   close_doc   },
	{ "next_signal",  (PyCFunction)class_signalfd_nsi,  METH_VARARGS | METH_KEYWORDS,  nsignal_doc },
	{ "set_sigset",   (PyCFunction)class_signalfd_ssi,  METH_VARARGS,                  ssigset_doc },
	{ "fileno",       (PyCFunction)class_signalfd_fno,  METH_NOARGS,                   fileno_doc  },
	{ NULL, NULL, 0, NULL },
};

static PyTypeObject class_signalfd = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".SignalFD",		/* tp_name */
	sizeof(object_signalfd),		/* tp_basicsize */
	0,					/* tp_itemsize */
	(destructor)class_signalfd_del,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_reserved */
	0,					/* tp_repr */
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
	(const char *)class_signalfd_doc,	/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	class_signalfd_methods,			/* tp_methods */
	class_signalfd_members,			/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	(initproc)class_signalfd_ini,		/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	class_signalfd_new,			/* tp_new */
	PyObject_Del,				/* tp_free */
};

static PyObject *class_signalfd_clo(object_signalfd *self, PyObject *args, PyObject *kwds)
{
	if(-1 != self->fileno)
	{
		close(self->fileno);
		self->fileno = -1;
	}
	Py_RETURN_NONE;
}

static void class_signalfd_del(object_signalfd *self)
{
	close(self->fileno);
	Py_XDECREF(self->sigset);
	PyObject_Del(self);
	return;
}

static PyObject *class_signalfd_fno(object_signalfd *self, PyObject *args, PyObject *kwds)
{
	if(-1 == self->fileno)
		Py_RETURN_NONE;
	return PyLong_FromLong(self->fileno);
}

static int class_signalfd_ini(object_signalfd *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "sigset", "flags", NULL };

	sigset_t    sigmask[1];
	PyObject   *asigset = NULL;
	PyObject   *ssigset = NULL;
	int         aflags  = 0;
	int         fd;
	int         retv = -1;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", (char **)keywords, &asigset, &aflags))
		goto fail;
	if(NULL == (ssigset = PySet_New(asigset)))
		goto fail;
	if(-1   == sigmask_from_iterable(ssigset, sigmask))
		goto fail;
	if(-1   == (fd = signalfd(self->fileno, sigmask, aflags)))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		goto fail;
	}
	Py_XDECREF(self->sigset);
	self->fileno = fd;
	self->sigset = PyFrozenSet_New(ssigset);
	self->flags  = aflags;
	(void)memcpy(&(self->sigmsk), sigmask, sizeof(sigset_t));
	retv = 0;
fail:
	if(NULL != ssigset)
		Py_DECREF(ssigset);
	return retv;
}

static PyObject *class_signalfd_nsi(object_signalfd *self, PyObject *args, PyObject *kwds)
{
	object_signalinfo *retv;

	if(NULL == (retv = object_signalinfo_new()))
	{
		PyErr_SetString(PyExc_MemoryError, "Cannot allocate SignalInfo object");
		return NULL;
	}
	if(sizeof(struct signalfd_siginfo) != read(self->fileno, retv->signfo, sizeof(struct signalfd_siginfo)))
	{
		Py_DECREF(retv);
		if(EAGAIN != errno)
		{
			PyErr_SetFromErrno(PyExc_SystemError);
			return NULL;
		}
		Py_RETURN_NONE;
	}
	return (PyObject *)retv;
}

static PyObject *class_signalfd_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	object_signalfd *new = NULL;
	(void)args;
	(void)kwds;
	debug("Entering class_signalfd_new\n")
	if(NULL != (new = (object_signalfd *)type->tp_alloc(type, 0)))
	{
		Py_INCREF((PyObject *)Py_None);
		new->fileno = -1;
		new->sigset = Py_None;
		(void)sigemptyset(&(new->sigmsk));
		new->flags  = 0;
	}
	debug("Exiting class_signalfd_new\n")
	return (PyObject *)new;
}

static PyObject *class_signalfd_ssi(object_signalfd *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "sigset", NULL };
	
	int         fd;
	int         aflags;
	sigset_t    sigmask[1];
	PyObject   *asigset = NULL;
	PyObject   *ssigset = NULL;
	PyObject   *osigset = self->sigset;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", (char **)keywords, &asigset, &aflags))
		goto fail;
	if(NULL == (ssigset = PyFrozenSet_New(asigset)))
		goto fail;
	if(-1   == sigmask_from_iterable(ssigset, sigmask))
		goto fail;
	if(-1   == (fd = signalfd(self->fileno, sigmask, aflags))) {
		PyErr_SetFromErrno(PyExc_SystemError);
		goto fail;
	}
	Py_XDECREF(self->sigset);
	self->fileno  = fd;
	self->sigset = ssigset;
	self->flags  = aflags;
	(void)memcpy(&(self->sigmsk), sigmask, sizeof(sigset_t));
	return osigset;

fail:
	if(NULL != asigset)
	{
		while(0 <  PySet_GET_SIZE(asigset)) Py_DECREF(PySet_Pop(asigset));
		Py_DECREF(asigset);
	}
	if(NULL != ssigset)
	{
		while(0 <  PySet_GET_SIZE(ssigset)) Py_DECREF(PySet_Pop(ssigset));
		Py_DECREF(ssigset);
	}
	Py_RETURN_NONE;
}

/* SIGNALINFO class */

static PyGetSetDef class_signalinfo_attrs[] = {
	{ "ssi_signo",   (getter)class_signalinfo_gat, NULL, "Signal number",                      (void *)ATTR_SSI_SIGNO   },
	{ "ssi_errno",   (getter)class_signalinfo_gat, NULL, "Error number (unused)",              (void *)ATTR_SSI_ERRNO   },
	{ "ssi_code",    (getter)class_signalinfo_gat, NULL, "Signal code",                        (void *)ATTR_SSI_CODE    },
	{ "ssi_pid",     (getter)class_signalinfo_gat, NULL, "PID of sender",                      (void *)ATTR_SSI_PID     },
	{ "ssi_uid",     (getter)class_signalinfo_gat, NULL, "Real UID of sender",                 (void *)ATTR_SSI_UID     },
	{ "ssi_fd",      (getter)class_signalinfo_gat, NULL, "File descriptor (SIGIO)",            (void *)ATTR_SSI_FD      },
	{ "ssi_tid",     (getter)class_signalinfo_gat, NULL, "Kernel timer ID (POSIX timers)",     (void *)ATTR_SSI_TID     },
	{ "ssi_band",    (getter)class_signalinfo_gat, NULL, "Band event (SIGIO)",                 (void *)ATTR_SSI_BAND    },
	{ "ssi_overrun", (getter)class_signalinfo_gat, NULL, "POSIX timer overrun count",          (void *)ATTR_SSI_OVERRUN },
	{ "ssi_trapno",  (getter)class_signalinfo_gat, NULL, "Trap number that caused signal",     (void *)ATTR_SSI_TRAPNO  },
	{ "ssi_status",  (getter)class_signalinfo_gat, NULL, "Exit status or signal (SIGCHLD)",    (void *)ATTR_SSI_STATUS  },
	{ "ssi_int",     (getter)class_signalinfo_gat, NULL, "Integer sent by sigqueue(3)",        (void *)ATTR_SSI_INT     },
	{ "ssi_utime",   (getter)class_signalinfo_gat, NULL, "User CPU time consumed (SIGCHLD)",   (void *)ATTR_SSI_UTIME   },
	{ "ssi_stime",   (getter)class_signalinfo_gat, NULL, "System CPU time consumed (SIGCHLD)", (void *)ATTR_SSI_STIME   },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject class_signalinfo = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".SignalInfo",		/* tp_name */
	sizeof(object_signalinfo),		/* tp_basicsize */
	0,					/* tp_itemsize */
	(destructor)class_signalinfo_del,	/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_reserved */
	(reprfunc)class_signalinfo_rep,		/* tp_repr */
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
	(const char *)class_signalinfo_doc,	/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0,					/* tp_methods */
	0,					/* tp_members */
	class_signalinfo_attrs,			/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	//(initproc)class_signalinfo_ini,		/* tp_init */
	0,					/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	0,					/* tp_new */
	PyObject_Del,				/* tp_free */
};

static void class_signalinfo_del(object_signalinfo *self)
{
	union freelist_un *cur = (union freelist_un *)self->signfo;
	cur->next = signalfd_siginfo_freelist;
	signalfd_siginfo_freelist = cur;
	PyObject_Del(self);
	return;
}

static PyObject *class_signalinfo_gat(object_signalinfo *self, int attrcode)
{
/*
 * These values are precomputed jenkons hash codes for corresponding fields names
 */
	switch(attrcode)
	{
	case ATTR_SSI_BAND:	return Py_BuildValue("K", self->signfo->ssi_band   );
	case ATTR_SSI_CODE:	return Py_BuildValue("K", self->signfo->ssi_code   );
	case ATTR_SSI_ERRNO:	return Py_BuildValue("i", self->signfo->ssi_errno  );
	case ATTR_SSI_FD:	return Py_BuildValue("i", self->signfo->ssi_fd     );
	case ATTR_SSI_INT:	return Py_BuildValue("i", self->signfo->ssi_int    );
	case ATTR_SSI_OVERRUN:	return Py_BuildValue("K", self->signfo->ssi_overrun);
	case ATTR_SSI_PID:	return Py_BuildValue("K", self->signfo->ssi_pid    );
	case ATTR_SSI_SIGNO:	return Py_BuildValue("K", self->signfo->ssi_signo  );
	case ATTR_SSI_STATUS:	return Py_BuildValue("i", self->signfo->ssi_status );
	case ATTR_SSI_STIME:	return Py_BuildValue("k", self->signfo->ssi_stime  );
	case ATTR_SSI_TID:	return Py_BuildValue("K", self->signfo->ssi_tid    );
	case ATTR_SSI_TRAPNO:   return Py_BuildValue("K", self->signfo->ssi_trapno );
	case ATTR_SSI_UID:      return Py_BuildValue("K", self->signfo->ssi_uid    );
	case ATTR_SSI_UTIME:	return Py_BuildValue("k", self->signfo->ssi_utime  );
	default:
		break;
	}
	PyErr_Format(PyExc_AttributeError, "'%s' unknown attribute", Py_TYPE(self)->tp_name);
	return NULL;
}

static object_signalinfo *object_signalinfo_new(void)
{
	union freelist_un *nfo = signalfd_siginfo_freelist;
	PyObject          *new = NULL;

	if(NULL == nfo)
	{
		if(NULL == (nfo = (union freelist_un *)malloc(sizeof(union freelist_un))))
			goto fail;
		nfo->next = NULL;
	}
	if(NULL == (new = class_signalinfo.tp_alloc(&class_signalinfo, 0)))
		goto fail;
	signalfd_siginfo_freelist = nfo->next;
	((object_signalinfo *)new)->signfo = (struct signalfd_siginfo *)nfo;

fail:
	return (object_signalinfo *)new;
}

static PyObject *class_signalinfo_rep(object_signalinfo *self)
{
	struct signalfd_siginfo *info = self->signfo;
	return PyUnicode_FromFormat(
		" <%s object at %p>\n"
		" signo:   %lu\n"
		" errno:   %ld\n"
		" code:    %ld\n"
		" pid:     %lu\n"
		" uid:     %lu\n"
		" fd:      %ld\n"
		" tid:     %lu\n"
		" band:    %lu\n"
		" overrun: %lu\n"
		" trapno:  %lu\n"
		" status:  %ld\n"
		" int:     %ld\n"
		" utime:   %llu\n"
		" stime:   %llu\n",
		Py_TYPE(self)->tp_name, self,
		info->ssi_signo,
		info->ssi_errno,
		info->ssi_code,
		info->ssi_pid,
		info->ssi_uid,
		info->ssi_fd,
		info->ssi_tid,
		info->ssi_band,
		info->ssi_overrun,
		info->ssi_trapno,
		info->ssi_status,
		info->ssi_int,
		info->ssi_utime,
		info->ssi_stime,
		info->ssi_addr);
}

/* MASK manipulation */

static PyObject *frozenset_from_sigmask(sigset_t *sigmask)
{
	int       cursig;
	PyObject *frozenset;
	PyObject *setobj;

	debug(">> frozenset_from_sigmask\n")

	if(NULL == (setobj = PySet_New(NULL)))
		return NULL;

	for(cursig = 1; cursig <= _NSIG; cursig += 1)
	{
		if(1 == sigismember(sigmask, cursig))
		{
			PySet_Add(setobj, PyLong_FromLong((long)cursig));
		}
	}
	frozenset = PyFrozenSet_New(setobj);
	Py_DECREF(setobj);
	debug("<< frozenset_from_sigmask\n")
	return frozenset;
}

static int sigmask_from_iterable(PyObject *asigset, sigset_t *sigmask)
{
	PyObject *ssigset = NULL;

	debug(">> sigmask_from_iterable\n");
	if(NULL == (ssigset = PySet_New(asigset)))
	{
		goto fail;
	}
	(void)sigemptyset(sigmask);
	while(0 < PySet_GET_SIZE(ssigset))
	{
		PyObject *elem;
		long      snum;
		if(NULL == (elem = PySet_Pop(ssigset)))
		{
			PyErr_SetString(PyExc_RuntimeError, "sigmask_from_iterable: Internal error");
			goto fail;
		}
		if(!PyLong_Check(elem))
		{
			PyErr_SetString(PyExc_ValueError, "Signal number is not an integer");
			Py_DECREF(elem);
			goto fail;
		}
		snum = PyLong_AsLong(elem);
		Py_DECREF(elem);
		if(-1 == sigaddset(sigmask, (int)snum))
		{
			PyErr_SetFromErrno(PyExc_SystemError);
			goto fail;
		}
	}
	Py_DECREF(ssigset);
	debug("<< sigmask_from_iterable\n");
	return 0;
fail:
	debug("   sigmask_from_iterable (fail section)\n");
	if(NULL != ssigset)
	{
		while(0 <  PySet_GET_SIZE(ssigset)) Py_DECREF(PySet_Pop(ssigset));
		Py_DECREF(ssigset);
	}
	PyErr_SetString(PyExc_ValueError, "sigset: Bad signal number");
	debug("<< sigmask_from_iterable\n");
	return -1;
}

static PyObject *meth_sigpending (PyObject *module, PyObject *args, PyObject *kwds)
{
	sigset_t    pendings[1];

	debug(">> meth_sigpending(module = %p, args = %p, kwds = %p)\n", module, args, kwds);
	if(-1 == sigpending(pendings))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return NULL;
	}
	debug("<< meth_sigpending\n");
	return frozenset_from_sigmask(pendings);
}

static PyObject *meth_sigprocmask(PyObject *module, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "how", "sigset", NULL };

	int         ahow;
	PyObject   *asigset = NULL;
	PyObject   *ssigset = NULL;
	sigset_t    newmask[1];
	sigset_t    oldmask[1];

	debug(">> meth_sigprocmask\n");
	if(!PyArg_ParseTupleAndKeywords(args, kwds, "iO", (char **)keywords, &ahow, &asigset))	goto fail;
	if(NULL == (ssigset = PyFrozenSet_New(asigset)))					goto fail;
	if(-1 == sigmask_from_iterable(ssigset, newmask))				        goto fail;
	if(-1 == sigprocmask(ahow, newmask, oldmask))						goto fail;
	Py_DECREF(ssigset);
	debug("<< meth_sigprocmask\n");
	return frozenset_from_sigmask(oldmask);
fail:
	Py_DECREF(ssigset);
	return NULL;
}

static PyObject *meth_sigsuspend (PyObject *module, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "sigset", NULL };

	PyObject   *asigset = NULL;
	PyObject   *ssigset = NULL;
	sigset_t    sigsusp[1];

	debug(">> meth_sigsuspend");
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)keywords, &asigset))		goto fail;
	if(NULL == (ssigset = PyFrozenSet_New(asigset)))					goto fail;
	if(-1 == sigmask_from_iterable(ssigset, sigsusp))				        goto fail;
	if(-1 == sigsuspend(sigsusp))								goto fail;
	debug("<< meth_sigsuspend");
	Py_RETURN_NONE;
fail:
	Py_DECREF(ssigset);
	return NULL;
}

/*
 * Module initialization
 */
static int mod_sys_linux_signalfd_init(PyObject *module)
{
	if(-1 == PyModule_AddIntConstant(module, "SIG_BLOCK",    SIG_BLOCK   )) return 0;
	if(-1 == PyModule_AddIntConstant(module, "SIG_UNBLOCK",  SIG_UNBLOCK )) return 0;
	if(-1 == PyModule_AddIntConstant(module, "SIG_SETMASK",  SIG_SETMASK )) return 0;
	if(-1 == PyModule_AddIntConstant(module, "SFD_NONBLOCK", SFD_NONBLOCK)) return 0;
	if(-1 == PyModule_AddIntConstant(module, "SFD_CLOEXEC",  SFD_CLOEXEC )) return 0;
	if(-1 == PyModule_AddIntConstant(module, "SFD_NONBLOCK", SFD_NONBLOCK)) return 0;
	if(-1 == PyModule_AddIntConstant(module, "SFD_CLOEXEC",  SFD_CLOEXEC )) return 0;

	Py_TYPE(&class_signalfd) = &PyType_Type;
	if(-1 == PyType_Ready(&class_signalinfo))
		return 0;

	Py_INCREF((PyObject *)&class_signalfd);
	Py_INCREF((PyObject *)&class_signalinfo);
	if(0 != PyModule_AddObject(module, "SignalFD", (PyObject *)&class_signalfd))
		return 0;
	if(0 != PyModule_AddObject(module, "SignalInfo", (PyObject *)&class_signalinfo))
		return 0;

	return 1;
}

MODULE_SL_ADD_FUNCTION(sigpending,  METH_NOARGS);
MODULE_SL_ADD_FUNCTION(sigprocmask, METH_VARARGS | METH_KEYWORDS);
MODULE_SL_ADD_FUNCTION(sigsuspend,  METH_VARARGS | METH_KEYWORDS);
MODULE_SL_ADD_INITIALIZER(mod_sys_linux_signalfd_init)
