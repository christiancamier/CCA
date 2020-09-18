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

#include "Python.h"
#include "structmember.h"

#ifndef __MOD_SYS_LINUX_H__
#define __MOD_SYS_LINUX_H__

#define _MODULE_MACRO_CONCAT(x, y)	x##y
#define _MODULE_MACRO_STRING(x)		#x

#define MODULE_MACRO_CONCAT(x,y)	_MODULE_MACRO_CONCAT(x, y)
#define MODULE_MACRO_STRING(x)		_MODULE_MACRO_STRING(x)

#define MODULENAME_S	MODULE_MACRO_STRING(MODULENAME)

#define MODULE_PYINITIALIZER()		PyMODINIT_FUNC MODULE_MACRO_CONCAT(PyInit_, MODULENAME)(void)

#if defined(DEBUG)
# include <stdio.h>
# define debug(...)	{ fprintf(stderr, "DEBUG: "); fprintf(stderr, __VA_ARGS__); }
#else
# define debug(...)
#endif

struct functiondes_st {
	struct functiondes_st *next;
	PyMethodDef            fdes;
};

struct initializer_st {
	struct initializer_st  *next;
#if defined(DEBUG)
	const char             *fnam;
#endif
	int                   (*func)(PyObject *);
};

extern void module_sl_add_functiondes(struct functiondes_st *);
extern void module_sl_add_initializer(struct initializer_st *);

#define MODULE_SL_ADD_FUNCTION(name, flags)								\
static __attribute__((constructor)) void MODULE_MACRO_CONCAT(func_declare_, __LINE__)(void)		\
{													\
	static struct functiondes_st F = { 0, { #name, (PyCFunction)meth_##name, flags, name##_doc }};	\
	module_sl_add_functiondes(&F);									\
	return;												\
}

#define MODULE_SL_ADD_CONSTANT(name) if(-1 == (PyModule_AddIntConstant(module, #name, name))) return 0

#if defined(DEBUG)
#define MODULE_SL_ADD_INITIALIZER(initializer)					\
static __attribute__((constructor)) void constructor(void);			\
static void constructor(void) {							\
	static struct initializer_st I = { 0, #initializer, initializer };	\
	module_sl_add_initializer(&I);						\
	return;									\
}
#else
#define MODULE_SL_ADD_INITIALIZER(initializer)					\
static __attribute__((constructor)) void constructor(void);			\
static void constructor(void) {							\
	static struct initializer_st I = { 0, initializer };			\
	module_sl_add_initializer(&I);						\
	return;									\
}
#endif

#endif // ! __MOD_SYS_LINUX_H__
