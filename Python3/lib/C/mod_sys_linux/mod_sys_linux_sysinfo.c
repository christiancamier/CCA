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
#include "mod_sys_linux_sysinfo.h"
#include "mod_sys_linux_sysinfo.d"

static int       mod_sys_linux_sysinfo_init(PyObject *module);
static PyObject *class_sysinfo_gat(object_sysinfo *self, int attr_code);
static int       class_sysinfo_ini(object_sysinfo *self, PyObject *args, PyObject *kwds);
static PyObject *class_sysinfo_new(PyTypeObject   *type, PyObject *args, PyObject *kwds);
static PyObject *class_sysinfo_rep(object_sysinfo *self);

static PyGetSetDef class_sysinfo_attrs[] = {
	{ "uptime",	(getter)class_sysinfo_gat, NULL, "Seconds since boot",			(void *)ATTR_UPTIME	},
	{ "load_1",	(getter)class_sysinfo_gat, NULL, "1 minute load average",		(void *)ATTR_LOAD_1	},
	{ "load_5",	(getter)class_sysinfo_gat, NULL, "5 minute load average",		(void *)ATTR_LOAD_5	},
	{ "load_15",	(getter)class_sysinfo_gat, NULL, "15 minute load average",		(void *)ATTR_LOAD_15	},
	{ "loads",	(getter)class_sysinfo_gat, NULL, "tuple (load_1, load_5, load_15)",	(void *)ATTR_LOADS	},
	{ "totalram",	(getter)class_sysinfo_gat, NULL, "Total usable main memory size",	(void *)ATTR_TOTALRAM	},
	{ "freeram",	(getter)class_sysinfo_gat, NULL, "Available memory size",		(void *)ATTR_FREERAM	},
	{ "sharedram",	(getter)class_sysinfo_gat, NULL, "Amount of shared memory",		(void *)ATTR_SHAREDRAM	},
	{ "bufferram",	(getter)class_sysinfo_gat, NULL, "Memory used by buffers",		(void *)ATTR_BUFFERRAM	},
	{ "totalswap",	(getter)class_sysinfo_gat, NULL, "Total swap space size",		(void *)ATTR_TOTALSWAP  },
	{ "freeswap",	(getter)class_sysinfo_gat, NULL, "Swap space still available",		(void *)ATTR_FREESWAP	},
	{ "procs",	(getter)class_sysinfo_gat, NULL, "Number od current processes",		(void *)ATTR_PROCS	},
	{ "totalhigh",	(getter)class_sysinfo_gat, NULL, "Total high memory size",		(void *)ATTR_TOTALHIGH	},
	{ "freehigh",	(getter)class_sysinfo_gat, NULL, "Available high memory size",		(void *)ATTR_FREEHIGH	},
	{ "mem_unit",	(getter)class_sysinfo_gat, NULL, "Memory unit size in bytes",		(void *)ATTR_MEM_UNIT	},
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyTypeObject class_sysinfo = {
	PyVarObject_HEAD_INIT(0, 0)		/* Must fill in type value later */
	MODULENAME_S ".SysInfo",		/* tp_name */
	sizeof(object_sysinfo),			/* tp_basicsize */
	0,					/* tp_itemsize */
	0,					/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_reserved */
	(reprfunc)class_sysinfo_rep,		/* tp_repr */
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
	(const char *)class_sysinfo_doc,	/* tp_doc */
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
	(initproc)class_sysinfo_ini,		/* tp_init */
	PyType_GenericAlloc,			/* tp_alloc */
	class_sysinfo_new,			/* tp_new */
	PyObject_Del,				/* tp_free */
};

static PyObject *class_sysinfo_gat(object_sysinfo *self, int attr_code)
{
	switch(attr_code)
	{
	case ATTR_UPTIME:	return PyLong_FromLong(self->info->uptime);
	case ATTR_LOAD_1:	return PyLong_FromUnsignedLong(self->info->loads[0]);
	case ATTR_LOAD_5:	return PyLong_FromUnsignedLong(self->info->loads[1]);
	case ATTR_LOAD_15:	return PyLong_FromUnsignedLong(self->info->loads[2]);
	case ATTR_LOADS:	return PyTuple_Pack(3,
						    PyLong_FromUnsignedLong(self->info->loads[0]),
						    PyLong_FromUnsignedLong(self->info->loads[1]),
						    PyLong_FromUnsignedLong(self->info->loads[2]));
	case ATTR_TOTALRAM:	return PyLong_FromUnsignedLong(self->info->totalram);
	case ATTR_FREERAM:	return PyLong_FromUnsignedLong(self->info->freeram);
	case ATTR_SHAREDRAM:	return PyLong_FromUnsignedLong(self->info->sharedram);
	case ATTR_BUFFERRAM:	return PyLong_FromUnsignedLong(self->info->bufferram);
	case ATTR_TOTALSWAP:	return PyLong_FromUnsignedLong(self->info->totalswap);
	case ATTR_FREESWAP:	return PyLong_FromUnsignedLong(self->info->freeswap);
	case ATTR_PROCS:	return PyLong_FromUnsignedLong(self->info->procs);
	case ATTR_TOTALHIGH:	return PyLong_FromUnsignedLong(self->info->totalhigh);
	case ATTR_FREEHIGH:	return PyLong_FromUnsignedLong(self->info->freehigh);
	case ATTR_MEM_UNIT:	return PyLong_FromUnsignedLong((unsigned long)(self->info->mem_unit));
	default:
		break;
	}
	PyErr_Format(PyExc_AttributeError, "'%s' unknown attribute", Py_TYPE(self)->tp_name);
	return NULL;
}

static PyObject *class_sysinfo_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	object_sysinfo *new = NULL;
	debug(">> class_sysinfo_new\n");
	new = (object_sysinfo *)type->tp_alloc(type, 0);
	debug("<< class_sysinfo_new\n");
	return (PyObject *)new;
}

static int class_sysinfo_ini(object_sysinfo *self, PyObject *args, PyObject *kwds)
{
	if(-1 == (sysinfo(self->info)))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return 1;
	}
	return 0;
}

static PyObject *class_sysinfo_rep(object_sysinfo *self)
{
	struct sysinfo *info = self->info;

	return PyUnicode_FromFormat(
		"<%s object at %p>\n"
		" uptime:     %ld\n"
		" loads:      %lu %lu %lu\n"
		" total ram:  %lu\n"
		" free ram:   %lu\n"
		" shared ram: %lu\n"
		" total swap: %lu\n"
		" free swap:  %lu\n"
		" procs:      %u\n",
		Py_TYPE(self)->tp_name, self,
		info->uptime,
		info->loads[0], info->loads[1], info->loads[2],
		info->totalram,
		info->freeram,
		info->sharedram,
		info->totalswap,
		info->freeswap,
		info->procs);
}

static int mod_sys_linux_sysinfo_init(PyObject *module)
{
	if(-1 == PyType_Ready(&class_sysinfo))
		return 0;
	Py_INCREF((PyObject *)&class_sysinfo);
	if(0 != PyModule_AddObject(module, "SysInfo", (PyObject *)&class_sysinfo))
		return 0;
	return 1;
}

MODULE_SL_ADD_INITIALIZER(mod_sys_linux_sysinfo_init);
