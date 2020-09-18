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

#include <inttypes.h>
#include <string.h>

#include "mod_sys_linux.h"
#include "mod_sys_linux_uname.h"
#include "mod_sys_linux_uname.d"

static int       mod_sys_linux_uname_init(PyObject *module);
static PyObject *class_uname_gat(object_uname *self, int attrcode);
static int       class_uname_ini(object_uname *self, PyObject *args, PyObject *kwds);
static PyObject *class_uname_new(PyTypeObject   *type, PyObject *args, PyObject *kwds);
static PyObject *class_uname_rep(object_uname *self);

static PyGetSetDef class_sysinfo_attrs[] = {
	{"sysname",  (getter)class_uname_gat, NULL, "Operating system name.",                           (void *)ATTR_SYSNAME  },
	{"nodename", (getter)class_uname_gat, NULL, "Name within some implementation-defined network.", (void *)ATTR_NODENAME },
	{"release",  (getter)class_uname_gat, NULL, "Operating system release (e.g., \"2.6.28\").",     (void *)ATTR_RELEASE  },
	{"version",  (getter)class_uname_gat, NULL, "Operating system version.",                        (void *)ATTR_VERSION  },
	{"machine",  (getter)class_uname_gat, NULL, "Hardware identifier.",                             (void *)ATTR_MACHINE  },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject class_uname = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".Uname",			/* tp_name */
	sizeof(object_uname),			/* tp_basicsize */
	0,					/* tp_itemsize */
	0,					/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_reserved */
	(reprfunc)class_uname_rep,		/* tp_repr */
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
	(const char *)class_uname_doc,	        /* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0,					/* tp_methods */
	0,					/* tp_members */
	class_sysinfo_attrs,			/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	(initproc)class_uname_ini,		/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	class_uname_new,			/* tp_new */
	PyObject_Del,				/* tp_free */
};

static PyObject *class_uname_gat(object_uname *self, int attrcode)
{
	struct utsname *info = self->info;
	switch(attrcode)
	{
	case ATTR_SYSNAME:	return PyUnicode_FromString(info->sysname);
	case ATTR_NODENAME:	return PyUnicode_FromString(info->nodename);
	case ATTR_RELEASE:	return PyUnicode_FromString(info->release);
	case ATTR_VERSION:	return PyUnicode_FromString(info->version);
	case ATTR_MACHINE:	return PyUnicode_FromString(info->machine);
	default:
		break;
	}
	PyErr_Format(PyExc_AttributeError, "'%s' unknown attribute", Py_TYPE(self)->tp_name);
	return NULL;
}

static PyObject *class_uname_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	object_uname *new = NULL;
	debug(">> class_uname_new\n");
	new = (object_uname *)type->tp_alloc(type, 0);
	debug("<< class_uname_new\n");
	return (PyObject *)new;
}

static int class_uname_ini(object_uname *self, PyObject *args, PyObject *kwds)
{
	if(-1 == (uname(self->info)))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return 1;
	}
	return 0;
}

static PyObject *class_uname_rep(object_uname *self)
{
	struct utsname *info = self->info;

	return PyUnicode_FromFormat(
		"<%s object at %p>\n"
		" sysname:  %s\n"
		" nodename: %s\n"
		" release:  %s\n"
		" version:  %s\n"
		" machine:  %s\n",
		Py_TYPE(self)->tp_name, self,
		info->sysname,
		info->nodename,
		info->release,
		info->version,
		info->machine
		);
}

static int mod_sys_linux_uname_init(PyObject *module)
{
	if(-1 == PyType_Ready(&class_uname))
		return 0;
	Py_INCREF((PyObject *)&class_uname);
	if(0 != PyModule_AddObject(module, "Uname", (PyObject *)&class_uname))
		return 0;
	return 1;
}

MODULE_SL_ADD_INITIALIZER(mod_sys_linux_uname_init);
