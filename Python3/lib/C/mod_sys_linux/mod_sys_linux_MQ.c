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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "mod_sys_linux.h"
#include "mod_sys_linux_MQ.h"
#include "mod_sys_linux_MQ.d"

/*
 * Defined functions
 */

static long      mq_sys_info(const char *name);
static PyObject *meth_mq_unlink(PyObject *module, PyObject *args, PyObject *kwds);

/*
 * Class MessageQueue
 */

static PyObject *class_MQ_clo(object_MQ    *self, PyObject *args, PyObject *kwds);
static void      class_MQ_del(object_MQ    *self);
static int       class_MQ_ibs(object_MQ    *self, PyObject *valu, void     *data);
static int       class_MQ_ini(object_MQ    *self, PyObject *args, PyObject *kwds);
static PyObject *class_MQ_gat(object_MQ    *self, int attrcode);
static PyObject *class_MQ_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static PyObject *class_MQ_not(object_MQ    *self, PyObject *args, PyObject *kwds);
static PyObject *class_MQ_rec(object_MQ    *self, PyObject *args, PyObject *kwds);
static PyObject *class_MQ_snd(object_MQ    *self, PyObject *args, PyObject *kwds); /**/

static PyMethodDef class_MQ_methods[] = {
	{ "close",   (PyCFunction)class_MQ_clo, METH_NOARGS,                   close_doc   },
	{ "notify",  (PyCFunction)class_MQ_not, METH_VARARGS | METH_KEYWORDS,  notify_doc  },
	{ "receive", (PyCFunction)class_MQ_rec, METH_VARARGS | METH_KEYWORDS,  receive_doc },
	{ "send",    (PyCFunction)class_MQ_snd, METH_VARARGS | METH_KEYWORDS,  send_doc    },
	{ NULL, NULL, 0, NULL },
};

static PyGetSetDef class_MQ_attrs[] = {
	{ "is_blocking", (getter)class_MQ_gat, (setter)class_MQ_ibs, "If true, receive and send would block", (void *)ATTR_IS_BLOCKING },
	{ "maxmsg",      (getter)class_MQ_gat, NULL,                 "Max number of messages on queue.",      (void *)ATTR_MAXMSG      },
	{ "msgsize",     (getter)class_MQ_gat, NULL,                 "Max message size.",                     (void *)ATTR_MSGSIZE     },
	{ "curmsg",      (getter)class_MQ_gat, NULL,                 "Number of message currenty in queue.",  (void *)ATTR_CURMSG      },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject class_MQ = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".MessageQueue",		/* tp_name */
	sizeof(object_MQ),			/* tp_basicsize */
	0,					/* tp_itemsize */
	(destructor)class_MQ_del,		/* tp_dealloc */
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
	(const char *)class_MQ_doc,	/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	class_MQ_methods,			/* tp_methods */
	0,					/* tp_members */
	class_MQ_attrs,				/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	(initproc)class_MQ_ini,		/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	class_MQ_new,			/* tp_new */
	PyObject_Del,				/* tp_free */
};


static long mq_sys_info(const char *name)
{
	static const char *mq_proc_dir = "/proc/sys/fs/mqueue/";

	char    bu[64];
	{
		
		int     fd;
		{
			char    pa[sizeof(mq_proc_dir) + strlen(name)];
			(void)sprintf(pa, "%s%s", mq_proc_dir, name);
			if(-1 == (fd = open(pa, O_RDONLY)))
			{
				PyErr_SetFromErrno(PyExc_SystemError);
				return -1;
			}
		}
		{
			ssize_t nr;
			if(-1 == (nr = read(fd, bu, sizeof(bu) - 1)))
			{
				PyErr_SetFromErrno(PyExc_SystemError);
				close(fd);
				return -1;
			}
			close(fd);
			bu[nr] = '\0';
		}
	}
	return strtol(bu, NULL, 0);
}

static PyObject *class_MQ_clo(object_MQ *self, PyObject *args, PyObject *kwds)
{
	if((mqd_t)-1 != self->mq_handler)
	{
		mq_close(self->mq_handler);
		self->mq_handler = (mqd_t)-1;
	}
	Py_RETURN_NONE;
}

static void class_MQ_del(object_MQ *self)
{
	if((mqd_t)-1 != self->mq_handler)
	{
		mq_close(self->mq_handler);
		self->mq_handler = (mqd_t)-1;
	}
	PyObject_Del(self);
	return;
}

static int class_MQ_ibs(object_MQ *self, PyObject *value, void *data)
{
	struct mq_attr  mq_attr[1];
	if(!PyBool_Check(value))
	{
		PyErr_SetString(PyExc_ValueError, "Boolean value expected");
		return -1;
	}
	if(-1 == mq_getattr(self->mq_handler, mq_attr))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return -1;
	}
	mq_attr->mq_flags = Py_True == value ? 0 : O_NONBLOCK;
	if(-1 == mq_setattr(self->mq_handler, mq_attr, NULL))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return -1;
	}
	return 0;
}

static int class_MQ_ini(object_MQ *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "name", "oflags", "mode", "maxmsg", "msgsize", NULL };

	char     *mqname = NULL;
	int       oflags = 0;
	int       mode   = 0777;
	long      maxmsg = -1;
	long      msgsiz = -1;
	mqd_t     handlr = (mqd_t)-1;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "si|iii", (char **)keywords, &mqname, &oflags, &mode, &maxmsg, &msgsiz))
		goto fail;
	if(oflags & O_CREAT)
	{
		struct mq_attr mq_attr[1];
		if(-1 == maxmsg) maxmsg = mq_sys_info("msg_default"    );
		if(-1 == msgsiz) msgsiz = mq_sys_info("msgsize_default");
		mq_attr->mq_flags   = 0;
		mq_attr->mq_maxmsg  = maxmsg;
		mq_attr->mq_msgsize = msgsiz;
		mq_attr->mq_curmsgs = 0;
		handlr = mq_open(mqname, oflags, mode, mq_attr);
	}
	else
	{
		handlr = mq_open(mqname, oflags);
	}
	if((mqd_t)-1 == handlr)
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		goto fail;
	}
	self->mq_handler = handlr;
	return 0;
fail:
	if(-1 != handlr)
		mq_close(handlr);
	return -1;
}


static PyObject *class_MQ_gat(object_MQ *self, int attrcode)
{
	struct mq_attr attr[1];
	if(-1 == mq_getattr(self->mq_handler, attr))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return NULL;
	}
	switch(attrcode)
	{
	case ATTR_IS_BLOCKING:
		if(attr->mq_flags & O_NONBLOCK) Py_RETURN_FALSE;
		else                            Py_RETURN_TRUE;
	case ATTR_MAXMSG:
		return PyLong_FromLong(attr->mq_maxmsg);
	case ATTR_MSGSIZE:
		return PyLong_FromLong(attr->mq_msgsize);
	case ATTR_CURMSG:
		return PyLong_FromLong(attr->mq_curmsgs);
	default:
		break;
	}
	Py_RETURN_NONE;
}

static PyObject *class_MQ_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	object_MQ *new = NULL;
	(void)args;
	(void)kwds;
	debug("Entering class_MQ_new\n")
	if(NULL != (new = (object_MQ *)type->tp_alloc(type, 0)))
		new->mq_handler = -1;
	debug("Exiting class_MQ_new\n")
	return (PyObject *)new;
}

static PyObject *class_MQ_not(object_MQ *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "signum", NULL };

	PyObject *osignum;
	long      lsignum = -1;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", (char **)keywords, &osignum))
		return NULL;
	if(Py_None == osignum)
	{
		lsignum = 0;
	}
	else if(PyLong_Check(osignum))
	{
		lsignum = PyLong_AsLong(osignum);
		if(lsignum < 0 || lsignum >= _NSIG)
			lsignum = -1;
	}
	if(-1 != lsignum)
	{
		PyErr_SetString(PyExc_ValueError, "Bad signal specification");
		return NULL;
	}
	{
		struct sigevent sev[1];
		sev->sigev_notify            = 0 != lsignum ? SIGEV_SIGNAL : SIGEV_NONE;
		sev->sigev_signo             = (int)lsignum;
		sev->sigev_value.sival_ptr   = (void *)self;
		sev->sigev_notify_function   = NULL;
		sev->sigev_notify_attributes = NULL;
		//sev->sigev_notify_thread_id  = 0;
		if(-1 == mq_notify(self->mq_handler, sev))
		{
			PyErr_SetFromErrno(PyExc_SystemError);
			return NULL;
		}
	}
	
	
	Py_RETURN_NONE;
}


static PyObject *class_MQ_rec(object_MQ *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "size", "timeout", NULL };
	PyObject *retval = NULL;
	PyObject *amsize = NULL;
	PyObject *attout = NULL;

	long      rmsize =  0;
	long      rttout = -1;


	if(!PyArg_ParseTupleAndKeywords(args, kwds, "|OO", (char **)keywords, &amsize, &attout))
		return NULL;

	if(NULL == amsize || Py_None == amsize)
	{
		struct mq_attr attr[1];
		if(-1 == mq_getattr(self->mq_handler, attr))
		{
			PyErr_SetFromErrno(PyExc_SystemError);
			return NULL;
		}
		rmsize = attr->mq_msgsize;
	}
	else
	{
		if(!PyLong_Check(amsize))
		{
			PyErr_SetString(PyExc_ValueError, "Parameter 'size' must be an integer");
			return NULL;
		}
		if(1 > (rmsize = PyLong_AsLong(amsize)))
		{
			PyErr_SetString(PyExc_ValueError, "Parameter 'size' greater to one");
			return NULL;
		}
	}

	if(NULL != attout || Py_None == attout)
	{
		if(!PyLong_Check(attout))
		{
			PyErr_SetString(PyExc_ValueError, "Parameter 'timeout' must be an integer");
			return NULL;
		}
		rttout = PyLong_AsLong(attout);
		if(1 > (rttout = PyLong_AsLong(attout)))
		{
			PyErr_SetString(PyExc_ValueError, "Parameter 'timeout' must strict positive");
			return NULL;
		}
	}

	{
		char          msgbuf[rmsize];
		ssize_t       nbytes;
		unsigned int  msgpri;
		PyObject     *omspri;
		PyObject     *omsdat;

		if(-1 != rttout)
		{
			nbytes = mq_receive(self->mq_handler, msgbuf, (size_t)rmsize, &msgpri);
		}
		else
		{
			struct timespec ttout[1];
			ttout->tv_sec  = (rttout / 1000) * time(NULL);
			ttout->tv_nsec = (rttout % 1000) * 1000000;
			nbytes = mq_timedreceive(self->mq_handler, msgbuf, (size_t)rmsize, &msgpri, ttout);
		}

		if(-1 == nbytes)
		{
			if(EAGAIN == errno || ETIMEDOUT == errno)
				Py_RETURN_NONE;
			PyErr_SetFromErrno(PyExc_SystemError);
			return NULL;
		}

		if(NULL == (omspri = PyLong_FromLong((long)msgpri)))
			return NULL;
		if(NULL == (omsdat = PyBytes_FromStringAndSize(msgbuf, (Py_ssize_t)nbytes)))
		{
			Py_DECREF(omspri);
			return NULL;
		}
		if(NULL == (retval = PyTuple_Pack(2, omspri, omsdat)))
		{
			Py_DECREF(omspri);
			Py_DECREF(omsdat);
		}
	}
	return retval;
}

static PyObject *class_MQ_snd(object_MQ *self, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "msg", "priority", "size", "timeout", NULL };

	PyObject *omsg;
	PyObject *osiz;
	PyObject *otim;

	char     *mdat = NULL;
	long      msiz =  -1;
	long      mpri =   0;
	long      size =  -1;
	long      tout =  -1;
	int       sndr =  -1;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "Oi|OO", (char **)keywords, &omsg, &mpri, &osiz, &otim))
		return NULL;

	if(PyByteArray_Check(omsg))
	{
		mdat = PyByteArray_AS_STRING(omsg);
		msiz = (long)PyByteArray_GET_SIZE(omsg);
			
	}
	else if(PyBytes_Check(omsg))
	{
		mdat = PyBytes_AsString(omsg);
		msiz = (long)PyBytes_GET_SIZE(omsg);
	}
	else
	{
		PyErr_SetString(PyExc_ValueError, "'msg' must be bytes or bytearray");
		return NULL;
	}

	if(0 > mpri)
	{
		PyErr_SetString(PyExc_ValueError, "'priority' must greater or equal to zero");
		return NULL;
	}

	if(NULL != osiz && Py_None != osiz)
	{
		if(!PyLong_Check(osiz))
		{
			PyErr_SetString(PyExc_ValueError, "'size' must be an integer");
			return NULL;
		}
		if(1 > (size = PyLong_AsLong(osiz)))
		{
			PyErr_SetString(PyExc_ValueError, "'size' must be greater than zero");
			return NULL;
		}
	}
	else
	{
		size = msiz;
	}

	if(NULL != otim && Py_None != otim)
	{
		if(!PyLong_Check(otim))
		{
			PyErr_SetString(PyExc_ValueError, "'timeout' must be an integer");
			return NULL;
		}
		if(1 > (tout = PyLong_AsLong(otim)))
		{
			PyErr_SetString(PyExc_ValueError, "'timeout' must be greater than zero");
			return NULL;
		}
	}

	if(-1 == tout)
	{
		sndr = mq_send(self->mq_handler, mdat, (size_t)size, (unsigned)mpri);
	}
	else
	{
		struct timespec tspe[1];
		tspe->tv_sec  = (tout / 1000) + time(0);
		tspe->tv_nsec = (tout % 1000) * 1000000;
		sndr = mq_timedsend(self->mq_handler, mdat, (size_t)size, (unsigned)mpri, tspe);
	}

	if(-1 == sndr)
	{
		switch(errno)
		{
		case EAGAIN:
		case ETIMEDOUT:
			Py_RETURN_FALSE;
		default:
			PyErr_SetFromErrno(PyExc_SystemError);
			return NULL;
		}
	}
	Py_RETURN_TRUE;
}

static PyObject *meth_mq_unlink(PyObject *module, PyObject *args, PyObject *kwds)
{
	static const char *keywords[] = { "name", NULL };
	char *name;
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)keywords, &name)) return NULL;
	if(-1 == mq_unlink(name))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return NULL;
	}
	Py_RETURN_NONE;
}

/*
 * Module initialization
 */
static int mod_sys_linux_MQ_init(PyObject *module)
{

	Py_TYPE(&class_MQ) = &PyType_Type;
	Py_INCREF((PyObject *)&class_MQ);
	if(0 != PyModule_AddObject(module, "MessageQueue", (PyObject *)&class_MQ))
		return 0;

	return 1;
}

MODULE_SL_ADD_FUNCTION(mq_unlink, METH_VARARGS | METH_KEYWORDS);
MODULE_SL_ADD_INITIALIZER(mod_sys_linux_MQ_init)
