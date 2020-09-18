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

#include <stdlib.h>

#include "mod_sys_linux.h"
#include "mod_sys_linux.d"

extern void module_sl_add_functiondes(struct functiondes_st *);
extern void module_sl_add_initializer(struct initializer_st *);

static struct initializer_st  initializer_dummy = { NULL };
static struct initializer_st *initializer_first = &initializer_dummy;
static struct initializer_st *initializer_last  = &initializer_dummy;
static struct functiondes_st  functiondes_dummy = {NULL, {NULL, NULL, 0, NULL }};
static struct functiondes_st *functiondes_list  = &functiondes_dummy;
static size_t                 functiondes_count = 1;


/*
 * Module definition
 */
static struct PyModuleDef SysLinuxModule = {
	PyModuleDef_HEAD_INIT,	/* m_base: Module header */
	MODULENAME_S,		/* m_name: Module name   */
	module_doc,		/* m_doc:  Module documentation */
	-1,			/* m_size: unused */
	NULL,			/* m_methods: Methods (filled later) */
	NULL,			/* m_slots: unused */
	NULL,			/* m_traverse: unused */
	NULL,			/* m_clear: unused */
	NULL			/* m_free: unused */
};

/*
 * Function module_sl_add_functiondes description
 *
 * This function is called by constructors to adds initializer.
 */
void module_sl_add_functiondes(struct functiondes_st *functiondes)
{
	functiondes->next  = functiondes_list;
	functiondes_list   = functiondes;
	functiondes_count += 1;
	return;
}

/*
 * Function module_sl_add_initializer
 *
 * This function is called by constructors to adds initializer.
 */
void module_sl_add_initializer(struct initializer_st *initializer)
{
	initializer->next      = NULL;
	initializer_last->next = initializer;
	initializer_last       = initializer;
	return;
}

/*
 * Function PyInit_mod_sys_linux:
 *
 * This function initialize the python module.
 */
MODULE_PYINITIALIZER()
{
	PyObject              *module;
	if(1)
	{
		PyMethodDef           *met;
		PyMethodDef           *dst;
		struct functiondes_st *src;
		if(NULL == (met = (PyMethodDef *)malloc(sizeof(PyMethodDef) * functiondes_count)))
		{
			PyErr_SetString(PyExc_MemoryError, MODULENAME_S ": Cannot allocate memory for methods definitions");
			return NULL;
		}
		for(dst = met, src = functiondes_list; src; dst += 1, src = src->next)
		{
			dst->ml_name  = src->fdes.ml_name;
			dst->ml_meth  = src->fdes.ml_meth;
			dst->ml_flags = src->fdes.ml_flags;
			dst->ml_doc   = src->fdes.ml_doc;
		}
		SysLinuxModule.m_methods = met;
	}
	if(NULL == (module = PyModule_Create(&SysLinuxModule)))
		return NULL;
	if(1)
	{
		struct initializer_st *pinit;
		for(pinit = initializer_first->next; NULL != pinit; pinit = pinit->next)
		{
			debug("Calling '%s' initiator\n", pinit->fnam);
			if(!pinit->func(module))
				return NULL;
		}
	}
	return module;
}
