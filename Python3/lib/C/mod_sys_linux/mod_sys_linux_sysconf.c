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
#include <unistd.h>

#include "mod_sys_linux.h"
#include "mod_sys_linux_sysconf.h"
#include "mod_sys_linux_sysconf.d"

static int mod_sys_linux_sysconf_init(PyObject *module);
static PyObject *meth_pathconf(PyObject *module, PyObject *args, PyObject*kwds);
static PyObject *meth_sysconf (PyObject *module, PyObject *args, PyObject*kwds);

static PyObject *meth_pathconf(PyObject *module, PyObject *args, PyObject*kwds)
{
	static const char *keywords[] = { "path", "name", NULL };

	char *apath = NULL;
	int   aname = 0;
	long  retv  = 0;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "si", (char **)keywords, &apath, &aname))
		return NULL;
	if(-1 == (retv = pathconf(apath, aname)))
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return NULL;
	}
	return PyLong_FromLong(retv);
}

static PyObject *meth_sysconf (PyObject *module, PyObject *args, PyObject*kwds)
{
	static const char *keywords[] = { "name", NULL };

	int   aname = 0;
	long  retv  = 0;

	if(!PyArg_ParseTupleAndKeywords(args, kwds, "i", (char **)keywords, &aname))
		return NULL;
	if(-1 == (retv = sysconf(aname)) && 0 != errno)
	{
		PyErr_SetFromErrno(PyExc_SystemError);
		return NULL;
	}
	return PyLong_FromLong(retv);
}

static int mod_sys_linux_sysconf_init(PyObject *module)
{
#if defined(_PC_LINK_MAX)
	if(-1 == PyModule_AddIntConstant(module, "PC_LINK_MAX", _PC_LINK_MAX))
		return 0;
#endif
#if defined(_PC_MAX_CANON)
	if(-1 == PyModule_AddIntConstant(module, "PC_MAX_CANON", _PC_MAX_CANON))
		return 0;
#endif
#if defined(_PC_MAX_INPUT)
	if(-1 == PyModule_AddIntConstant(module, "PC_MAX_INPUT", _PC_MAX_INPUT))
		return 0;
#endif
#if defined(_PC_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "PC_NAME_MAX", _PC_NAME_MAX))
		return 0;
#endif
#if defined(_PC_PATH_MAX)
	if(-1 == PyModule_AddIntConstant(module, "PC_PATH_MAX", _PC_PATH_MAX))
		return 0;
#endif
#if defined(_PC_PIPE_BUF)
	if(-1 == PyModule_AddIntConstant(module, "PC_PIPE_BUF", _PC_PIPE_BUF))
		return 0;
#endif
#if defined(_PC_CHOWN_RESTRICTED)
	if(-1 == PyModule_AddIntConstant(module, "PC_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED))
		return 0;
#endif
#if defined(_PC_NO_TRUNC)
	if(-1 == PyModule_AddIntConstant(module, "PC_NO_TRUNC", _PC_NO_TRUNC))
		return 0;
#endif
#if defined(_PC_VDISABLE)
	if(-1 == PyModule_AddIntConstant(module, "PC_VDISABLE", _PC_VDISABLE))
		return 0;
#endif
#if defined(_PC_SYNC_IO)
	if(-1 == PyModule_AddIntConstant(module, "PC_SYNC_IO", _PC_SYNC_IO))
		return 0;
#endif
#if defined(_PC_ASYNC_IO)
	if(-1 == PyModule_AddIntConstant(module, "PC_ASYNC_IO", _PC_ASYNC_IO))
		return 0;
#endif
#if defined(_PC_PRIO_IO)
	if(-1 == PyModule_AddIntConstant(module, "PC_PRIO_IO", _PC_PRIO_IO))
		return 0;
#endif
#if defined(_PC_SOCK_MAXBUF)
	if(-1 == PyModule_AddIntConstant(module, "PC_SOCK_MAXBUF", _PC_SOCK_MAXBUF))
		return 0;
#endif
#if defined(_PC_FILESIZEBITS)
	if(-1 == PyModule_AddIntConstant(module, "PC_FILESIZEBITS", _PC_FILESIZEBITS))
		return 0;
#endif
#if defined(_PC_REC_INCR_XFER_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "PC_REC_INCR_XFER_SIZE", _PC_REC_INCR_XFER_SIZE))
		return 0;
#endif
#if defined(_PC_REC_MAX_XFER_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "PC_REC_MAX_XFER_SIZE", _PC_REC_MAX_XFER_SIZE))
		return 0;
#endif
#if defined(_PC_REC_MIN_XFER_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "PC_REC_MIN_XFER_SIZE", _PC_REC_MIN_XFER_SIZE))
		return 0;
#endif
#if defined(_PC_REC_XFER_ALIGN)
	if(-1 == PyModule_AddIntConstant(module, "PC_REC_XFER_ALIGN", _PC_REC_XFER_ALIGN))
		return 0;
#endif
#if defined(_PC_ALLOC_SIZE_MIN)
	if(-1 == PyModule_AddIntConstant(module, "PC_ALLOC_SIZE_MIN", _PC_ALLOC_SIZE_MIN))
		return 0;
#endif
#if defined(_PC_SYMLINK_MAX)
	if(-1 == PyModule_AddIntConstant(module, "PC_SYMLINK_MAX", _PC_SYMLINK_MAX))
		return 0;
#endif
#if defined(_PC_2_SYMLINKS)
	if(-1 == PyModule_AddIntConstant(module, "PC_2_SYMLINKS", _PC_2_SYMLINKS))
		return 0;
#endif
#if defined(_SC_ARG_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_ARG_MAX", _SC_ARG_MAX))
		return 0;
#endif
#if defined(_SC_CHILD_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_CHILD_MAX", _SC_CHILD_MAX))
		return 0;
#endif
#if defined(_SC_CLK_TCK)
	if(-1 == PyModule_AddIntConstant(module, "SC_CLK_TCK", _SC_CLK_TCK))
		return 0;
#endif
#if defined(_SC_NGROUPS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NGROUPS_MAX", _SC_NGROUPS_MAX))
		return 0;
#endif
#if defined(_SC_OPEN_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_OPEN_MAX", _SC_OPEN_MAX))
		return 0;
#endif
#if defined(_SC_STREAM_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_STREAM_MAX", _SC_STREAM_MAX))
		return 0;
#endif
#if defined(_SC_TZNAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TZNAME_MAX", _SC_TZNAME_MAX))
		return 0;
#endif
#if defined(_SC_JOB_CONTROL)
	if(-1 == PyModule_AddIntConstant(module, "SC_JOB_CONTROL", _SC_JOB_CONTROL))
		return 0;
#endif
#if defined(_SC_SAVED_IDS)
	if(-1 == PyModule_AddIntConstant(module, "SC_SAVED_IDS", _SC_SAVED_IDS))
		return 0;
#endif
#if defined(_SC_REALTIME_SIGNALS)
	if(-1 == PyModule_AddIntConstant(module, "SC_REALTIME_SIGNALS", _SC_REALTIME_SIGNALS))
		return 0;
#endif
#if defined(_SC_PRIORITY_SCHEDULING)
	if(-1 == PyModule_AddIntConstant(module, "SC_PRIORITY_SCHEDULING", _SC_PRIORITY_SCHEDULING))
		return 0;
#endif
#if defined(_SC_TIMERS)
	if(-1 == PyModule_AddIntConstant(module, "SC_TIMERS", _SC_TIMERS))
		return 0;
#endif
#if defined(_SC_ASYNCHRONOUS_IO)
	if(-1 == PyModule_AddIntConstant(module, "SC_ASYNCHRONOUS_IO", _SC_ASYNCHRONOUS_IO))
		return 0;
#endif
#if defined(_SC_PRIORITIZED_IO)
	if(-1 == PyModule_AddIntConstant(module, "SC_PRIORITIZED_IO", _SC_PRIORITIZED_IO))
		return 0;
#endif
#if defined(_SC_SYNCHRONIZED_IO)
	if(-1 == PyModule_AddIntConstant(module, "SC_SYNCHRONIZED_IO", _SC_SYNCHRONIZED_IO))
		return 0;
#endif
#if defined(_SC_FSYNC)
	if(-1 == PyModule_AddIntConstant(module, "SC_FSYNC", _SC_FSYNC))
		return 0;
#endif
#if defined(_SC_MAPPED_FILES)
	if(-1 == PyModule_AddIntConstant(module, "SC_MAPPED_FILES", _SC_MAPPED_FILES))
		return 0;
#endif
#if defined(_SC_MEMLOCK)
	if(-1 == PyModule_AddIntConstant(module, "SC_MEMLOCK", _SC_MEMLOCK))
		return 0;
#endif
#if defined(_SC_MEMLOCK_RANGE)
	if(-1 == PyModule_AddIntConstant(module, "SC_MEMLOCK_RANGE", _SC_MEMLOCK_RANGE))
		return 0;
#endif
#if defined(_SC_MEMORY_PROTECTION)
	if(-1 == PyModule_AddIntConstant(module, "SC_MEMORY_PROTECTION", _SC_MEMORY_PROTECTION))
		return 0;
#endif
#if defined(_SC_MESSAGE_PASSING)
	if(-1 == PyModule_AddIntConstant(module, "SC_MESSAGE_PASSING", _SC_MESSAGE_PASSING))
		return 0;
#endif
#if defined(_SC_SEMAPHORES)
	if(-1 == PyModule_AddIntConstant(module, "SC_SEMAPHORES", _SC_SEMAPHORES))
		return 0;
#endif
#if defined(_SC_SHARED_MEMORY_OBJECTS)
	if(-1 == PyModule_AddIntConstant(module, "SC_SHARED_MEMORY_OBJECTS", _SC_SHARED_MEMORY_OBJECTS))
		return 0;
#endif
#if defined(_SC_AIO_LISTIO_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_AIO_LISTIO_MAX", _SC_AIO_LISTIO_MAX))
		return 0;
#endif
#if defined(_SC_AIO_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_AIO_MAX", _SC_AIO_MAX))
		return 0;
#endif
#if defined(_SC_AIO_PRIO_DELTA_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_AIO_PRIO_DELTA_MAX", _SC_AIO_PRIO_DELTA_MAX))
		return 0;
#endif
#if defined(_SC_DELAYTIMER_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_DELAYTIMER_MAX", _SC_DELAYTIMER_MAX))
		return 0;
#endif
#if defined(_SC_MQ_OPEN_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_MQ_OPEN_MAX", _SC_MQ_OPEN_MAX))
		return 0;
#endif
#if defined(_SC_MQ_PRIO_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_MQ_PRIO_MAX", _SC_MQ_PRIO_MAX))
		return 0;
#endif
#if defined(_SC_VERSION)
	if(-1 == PyModule_AddIntConstant(module, "SC_VERSION", _SC_VERSION))
		return 0;
#endif
#if defined(_SC_PAGESIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_PAGESIZE", _SC_PAGESIZE))
		return 0;
#endif
#if defined(_SC_PAGE_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_PAGE_SIZE", _SC_PAGE_SIZE))
		return 0;
#endif
#if defined(_SC_RTSIG_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_RTSIG_MAX", _SC_RTSIG_MAX))
		return 0;
#endif
#if defined(_SC_SEM_NSEMS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SEM_NSEMS_MAX", _SC_SEM_NSEMS_MAX))
		return 0;
#endif
#if defined(_SC_SEM_VALUE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SEM_VALUE_MAX", _SC_SEM_VALUE_MAX))
		return 0;
#endif
#if defined(_SC_SIGQUEUE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SIGQUEUE_MAX", _SC_SIGQUEUE_MAX))
		return 0;
#endif
#if defined(_SC_TIMER_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TIMER_MAX", _SC_TIMER_MAX))
		return 0;
#endif
#if defined(_SC_BC_BASE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_BC_BASE_MAX", _SC_BC_BASE_MAX))
		return 0;
#endif
#if defined(_SC_BC_DIM_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_BC_DIM_MAX", _SC_BC_DIM_MAX))
		return 0;
#endif
#if defined(_SC_BC_SCALE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_BC_SCALE_MAX", _SC_BC_SCALE_MAX))
		return 0;
#endif
#if defined(_SC_BC_STRING_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_BC_STRING_MAX", _SC_BC_STRING_MAX))
		return 0;
#endif
#if defined(_SC_COLL_WEIGHTS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX))
		return 0;
#endif
#if defined(_SC_EQUIV_CLASS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_EQUIV_CLASS_MAX", _SC_EQUIV_CLASS_MAX))
		return 0;
#endif
#if defined(_SC_EXPR_NEST_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_EXPR_NEST_MAX", _SC_EXPR_NEST_MAX))
		return 0;
#endif
#if defined(_SC_LINE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_LINE_MAX", _SC_LINE_MAX))
		return 0;
#endif
#if defined(_SC_RE_DUP_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_RE_DUP_MAX", _SC_RE_DUP_MAX))
		return 0;
#endif
#if defined(_SC_CHARCLASS_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_CHARCLASS_NAME_MAX", _SC_CHARCLASS_NAME_MAX))
		return 0;
#endif
#if defined(_SC_2_VERSION)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_VERSION", _SC_2_VERSION))
		return 0;
#endif
#if defined(_SC_2_C_BIND)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_C_BIND", _SC_2_C_BIND))
		return 0;
#endif
#if defined(_SC_2_C_DEV)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_C_DEV", _SC_2_C_DEV))
		return 0;
#endif
#if defined(_SC_2_FORT_DEV)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_FORT_DEV", _SC_2_FORT_DEV))
		return 0;
#endif
#if defined(_SC_2_FORT_RUN)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_FORT_RUN", _SC_2_FORT_RUN))
		return 0;
#endif
#if defined(_SC_2_SW_DEV)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_SW_DEV", _SC_2_SW_DEV))
		return 0;
#endif
#if defined(_SC_2_LOCALEDEF)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_LOCALEDEF", _SC_2_LOCALEDEF))
		return 0;
#endif
#if defined(_SC_PII)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII", _SC_PII))
		return 0;
#endif
#if defined(_SC_PII_XTI)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_XTI", _SC_PII_XTI))
		return 0;
#endif
#if defined(_SC_PII_SOCKET)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_SOCKET", _SC_PII_SOCKET))
		return 0;
#endif
#if defined(_SC_PII_INTERNET)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_INTERNET", _SC_PII_INTERNET))
		return 0;
#endif
#if defined(_SC_PII_OSI)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_OSI", _SC_PII_OSI))
		return 0;
#endif
#if defined(_SC_POLL)
	if(-1 == PyModule_AddIntConstant(module, "SC_POLL", _SC_POLL))
		return 0;
#endif
#if defined(_SC_SELECT)
	if(-1 == PyModule_AddIntConstant(module, "SC_SELECT", _SC_SELECT))
		return 0;
#endif
#if defined(_SC_UIO_MAXIOV)
	if(-1 == PyModule_AddIntConstant(module, "SC_UIO_MAXIOV", _SC_UIO_MAXIOV))
		return 0;
#endif
#if defined(_SC_IOV_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_IOV_MAX", _SC_IOV_MAX))
		return 0;
#endif
#if defined(_SC_PII_INTERNET_STREAM)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_INTERNET_STREAM", _SC_PII_INTERNET_STREAM))
		return 0;
#endif
#if defined(_SC_PII_INTERNET_DGRAM)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_INTERNET_DGRAM", _SC_PII_INTERNET_DGRAM))
		return 0;
#endif
#if defined(_SC_PII_OSI_COTS)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_OSI_COTS", _SC_PII_OSI_COTS))
		return 0;
#endif
#if defined(_SC_PII_OSI_CLTS)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_OSI_CLTS", _SC_PII_OSI_CLTS))
		return 0;
#endif
#if defined(_SC_PII_OSI_M)
	if(-1 == PyModule_AddIntConstant(module, "SC_PII_OSI_M", _SC_PII_OSI_M))
		return 0;
#endif
#if defined(_SC_T_IOV_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_T_IOV_MAX", _SC_T_IOV_MAX))
		return 0;
#endif
#if defined(_SC_THREADS)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREADS", _SC_THREADS))
		return 0;
#endif
#if defined(_SC_THREAD_SAFE_FUNCTIONS)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_SAFE_FUNCTIONS", _SC_THREAD_SAFE_FUNCTIONS))
		return 0;
#endif
#if defined(_SC_GETGR_R_SIZE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_GETGR_R_SIZE_MAX", _SC_GETGR_R_SIZE_MAX))
		return 0;
#endif
#if defined(_SC_GETPW_R_SIZE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_GETPW_R_SIZE_MAX", _SC_GETPW_R_SIZE_MAX))
		return 0;
#endif
#if defined(_SC_LOGIN_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_LOGIN_NAME_MAX", _SC_LOGIN_NAME_MAX))
		return 0;
#endif
#if defined(_SC_TTY_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TTY_NAME_MAX", _SC_TTY_NAME_MAX))
		return 0;
#endif
#if defined(_SC_THREAD_DESTRUCTOR_ITERATIONS)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_DESTRUCTOR_ITERATIONS", _SC_THREAD_DESTRUCTOR_ITERATIONS))
		return 0;
#endif
#if defined(_SC_THREAD_KEYS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_KEYS_MAX", _SC_THREAD_KEYS_MAX))
		return 0;
#endif
#if defined(_SC_THREAD_STACK_MIN)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_STACK_MIN", _SC_THREAD_STACK_MIN))
		return 0;
#endif
#if defined(_SC_THREAD_THREADS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_THREADS_MAX", _SC_THREAD_THREADS_MAX))
		return 0;
#endif
#if defined(_SC_THREAD_ATTR_STACKADDR)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_ATTR_STACKADDR", _SC_THREAD_ATTR_STACKADDR))
		return 0;
#endif
#if defined(_SC_THREAD_ATTR_STACKSIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_ATTR_STACKSIZE", _SC_THREAD_ATTR_STACKSIZE))
		return 0;
#endif
#if defined(_SC_THREAD_PRIORITY_SCHEDULING)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_PRIORITY_SCHEDULING", _SC_THREAD_PRIORITY_SCHEDULING))
		return 0;
#endif
#if defined(_SC_THREAD_PRIO_INHERIT)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_PRIO_INHERIT", _SC_THREAD_PRIO_INHERIT))
		return 0;
#endif
#if defined(_SC_THREAD_PRIO_PROTECT)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_PRIO_PROTECT", _SC_THREAD_PRIO_PROTECT))
		return 0;
#endif
#if defined(_SC_THREAD_PROCESS_SHARED)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_PROCESS_SHARED", _SC_THREAD_PROCESS_SHARED))
		return 0;
#endif
#if defined(_SC_NPROCESSORS_CONF)
	if(-1 == PyModule_AddIntConstant(module, "SC_NPROCESSORS_CONF", _SC_NPROCESSORS_CONF))
		return 0;
#endif
#if defined(_SC_NPROCESSORS_ONLN)
	if(-1 == PyModule_AddIntConstant(module, "SC_NPROCESSORS_ONLN", _SC_NPROCESSORS_ONLN))
		return 0;
#endif
#if defined(_SC_PHYS_PAGES)
	if(-1 == PyModule_AddIntConstant(module, "SC_PHYS_PAGES", _SC_PHYS_PAGES))
		return 0;
#endif
#if defined(_SC_AVPHYS_PAGES)
	if(-1 == PyModule_AddIntConstant(module, "SC_AVPHYS_PAGES", _SC_AVPHYS_PAGES))
		return 0;
#endif
#if defined(_SC_ATEXIT_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_ATEXIT_MAX", _SC_ATEXIT_MAX))
		return 0;
#endif
#if defined(_SC_PASS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_PASS_MAX", _SC_PASS_MAX))
		return 0;
#endif
#if defined(_SC_XOPEN_VERSION)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_VERSION", _SC_XOPEN_VERSION))
		return 0;
#endif
#if defined(_SC_XOPEN_XCU_VERSION)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_XCU_VERSION", _SC_XOPEN_XCU_VERSION))
		return 0;
#endif
#if defined(_SC_XOPEN_UNIX)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_UNIX", _SC_XOPEN_UNIX))
		return 0;
#endif
#if defined(_SC_XOPEN_CRYPT)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_CRYPT", _SC_XOPEN_CRYPT))
		return 0;
#endif
#if defined(_SC_XOPEN_ENH_I18N)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_ENH_I18N", _SC_XOPEN_ENH_I18N))
		return 0;
#endif
#if defined(_SC_XOPEN_SHM)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_SHM", _SC_XOPEN_SHM))
		return 0;
#endif
#if defined(_SC_2_CHAR_TERM)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_CHAR_TERM", _SC_2_CHAR_TERM))
		return 0;
#endif
#if defined(_SC_2_C_VERSION)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_C_VERSION", _SC_2_C_VERSION))
		return 0;
#endif
#if defined(_SC_2_UPE)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_UPE", _SC_2_UPE))
		return 0;
#endif
#if defined(_SC_XOPEN_XPG2)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_XPG2", _SC_XOPEN_XPG2))
		return 0;
#endif
#if defined(_SC_XOPEN_XPG3)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_XPG3", _SC_XOPEN_XPG3))
		return 0;
#endif
#if defined(_SC_XOPEN_XPG4)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_XPG4", _SC_XOPEN_XPG4))
		return 0;
#endif
#if defined(_SC_CHAR_BIT)
	if(-1 == PyModule_AddIntConstant(module, "SC_CHAR_BIT", _SC_CHAR_BIT))
		return 0;
#endif
#if defined(_SC_CHAR_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_CHAR_MAX", _SC_CHAR_MAX))
		return 0;
#endif
#if defined(_SC_CHAR_MIN)
	if(-1 == PyModule_AddIntConstant(module, "SC_CHAR_MIN", _SC_CHAR_MIN))
		return 0;
#endif
#if defined(_SC_INT_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_INT_MAX", _SC_INT_MAX))
		return 0;
#endif
#if defined(_SC_INT_MIN)
	if(-1 == PyModule_AddIntConstant(module, "SC_INT_MIN", _SC_INT_MIN))
		return 0;
#endif
#if defined(_SC_LONG_BIT)
	if(-1 == PyModule_AddIntConstant(module, "SC_LONG_BIT", _SC_LONG_BIT))
		return 0;
#endif
#if defined(_SC_WORD_BIT)
	if(-1 == PyModule_AddIntConstant(module, "SC_WORD_BIT", _SC_WORD_BIT))
		return 0;
#endif
#if defined(_SC_MB_LEN_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_MB_LEN_MAX", _SC_MB_LEN_MAX))
		return 0;
#endif
#if defined(_SC_NZERO)
	if(-1 == PyModule_AddIntConstant(module, "SC_NZERO", _SC_NZERO))
		return 0;
#endif
#if defined(_SC_SSIZE_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SSIZE_MAX", _SC_SSIZE_MAX))
		return 0;
#endif
#if defined(_SC_SCHAR_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SCHAR_MAX", _SC_SCHAR_MAX))
		return 0;
#endif
#if defined(_SC_SCHAR_MIN)
	if(-1 == PyModule_AddIntConstant(module, "SC_SCHAR_MIN", _SC_SCHAR_MIN))
		return 0;
#endif
#if defined(_SC_SHRT_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SHRT_MAX", _SC_SHRT_MAX))
		return 0;
#endif
#if defined(_SC_SHRT_MIN)
	if(-1 == PyModule_AddIntConstant(module, "SC_SHRT_MIN", _SC_SHRT_MIN))
		return 0;
#endif
#if defined(_SC_UCHAR_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_UCHAR_MAX", _SC_UCHAR_MAX))
		return 0;
#endif
#if defined(_SC_UINT_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_UINT_MAX", _SC_UINT_MAX))
		return 0;
#endif
#if defined(_SC_ULONG_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_ULONG_MAX", _SC_ULONG_MAX))
		return 0;
#endif
#if defined(_SC_USHRT_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_USHRT_MAX", _SC_USHRT_MAX))
		return 0;
#endif
#if defined(_SC_NL_ARGMAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NL_ARGMAX", _SC_NL_ARGMAX))
		return 0;
#endif
#if defined(_SC_NL_LANGMAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NL_LANGMAX", _SC_NL_LANGMAX))
		return 0;
#endif
#if defined(_SC_NL_MSGMAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NL_MSGMAX", _SC_NL_MSGMAX))
		return 0;
#endif
#if defined(_SC_NL_NMAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NL_NMAX", _SC_NL_NMAX))
		return 0;
#endif
#if defined(_SC_NL_SETMAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NL_SETMAX", _SC_NL_SETMAX))
		return 0;
#endif
#if defined(_SC_NL_TEXTMAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_NL_TEXTMAX", _SC_NL_TEXTMAX))
		return 0;
#endif
#if defined(_SC_XBS5_ILP32_OFF32)
	if(-1 == PyModule_AddIntConstant(module, "SC_XBS5_ILP32_OFF32", _SC_XBS5_ILP32_OFF32))
		return 0;
#endif
#if defined(_SC_XBS5_ILP32_OFFBIG)
	if(-1 == PyModule_AddIntConstant(module, "SC_XBS5_ILP32_OFFBIG", _SC_XBS5_ILP32_OFFBIG))
		return 0;
#endif
#if defined(_SC_XBS5_LP64_OFF64)
	if(-1 == PyModule_AddIntConstant(module, "SC_XBS5_LP64_OFF64", _SC_XBS5_LP64_OFF64))
		return 0;
#endif
#if defined(_SC_XBS5_LPBIG_OFFBIG)
	if(-1 == PyModule_AddIntConstant(module, "SC_XBS5_LPBIG_OFFBIG", _SC_XBS5_LPBIG_OFFBIG))
		return 0;
#endif
#if defined(_SC_XOPEN_LEGACY)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_LEGACY", _SC_XOPEN_LEGACY))
		return 0;
#endif
#if defined(_SC_XOPEN_REALTIME)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_REALTIME", _SC_XOPEN_REALTIME))
		return 0;
#endif
#if defined(_SC_XOPEN_REALTIME_THREADS)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_REALTIME_THREADS", _SC_XOPEN_REALTIME_THREADS))
		return 0;
#endif
#if defined(_SC_ADVISORY_INFO)
	if(-1 == PyModule_AddIntConstant(module, "SC_ADVISORY_INFO", _SC_ADVISORY_INFO))
		return 0;
#endif
#if defined(_SC_BARRIERS)
	if(-1 == PyModule_AddIntConstant(module, "SC_BARRIERS", _SC_BARRIERS))
		return 0;
#endif
#if defined(_SC_BASE)
	if(-1 == PyModule_AddIntConstant(module, "SC_BASE", _SC_BASE))
		return 0;
#endif
#if defined(_SC_C_LANG_SUPPORT)
	if(-1 == PyModule_AddIntConstant(module, "SC_C_LANG_SUPPORT", _SC_C_LANG_SUPPORT))
		return 0;
#endif
#if defined(_SC_C_LANG_SUPPORT_R)
	if(-1 == PyModule_AddIntConstant(module, "SC_C_LANG_SUPPORT_R", _SC_C_LANG_SUPPORT_R))
		return 0;
#endif
#if defined(_SC_CLOCK_SELECTION)
	if(-1 == PyModule_AddIntConstant(module, "SC_CLOCK_SELECTION", _SC_CLOCK_SELECTION))
		return 0;
#endif
#if defined(_SC_CPUTIME)
	if(-1 == PyModule_AddIntConstant(module, "SC_CPUTIME", _SC_CPUTIME))
		return 0;
#endif
#if defined(_SC_THREAD_CPUTIME)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_CPUTIME", _SC_THREAD_CPUTIME))
		return 0;
#endif
#if defined(_SC_DEVICE_IO)
	if(-1 == PyModule_AddIntConstant(module, "SC_DEVICE_IO", _SC_DEVICE_IO))
		return 0;
#endif
#if defined(_SC_DEVICE_SPECIFIC)
	if(-1 == PyModule_AddIntConstant(module, "SC_DEVICE_SPECIFIC", _SC_DEVICE_SPECIFIC))
		return 0;
#endif
#if defined(_SC_DEVICE_SPECIFIC_R)
	if(-1 == PyModule_AddIntConstant(module, "SC_DEVICE_SPECIFIC_R", _SC_DEVICE_SPECIFIC_R))
		return 0;
#endif
#if defined(_SC_FD_MGMT)
	if(-1 == PyModule_AddIntConstant(module, "SC_FD_MGMT", _SC_FD_MGMT))
		return 0;
#endif
#if defined(_SC_FIFO)
	if(-1 == PyModule_AddIntConstant(module, "SC_FIFO", _SC_FIFO))
		return 0;
#endif
#if defined(_SC_PIPE)
	if(-1 == PyModule_AddIntConstant(module, "SC_PIPE", _SC_PIPE))
		return 0;
#endif
#if defined(_SC_FILE_ATTRIBUTES)
	if(-1 == PyModule_AddIntConstant(module, "SC_FILE_ATTRIBUTES", _SC_FILE_ATTRIBUTES))
		return 0;
#endif
#if defined(_SC_FILE_LOCKING)
	if(-1 == PyModule_AddIntConstant(module, "SC_FILE_LOCKING", _SC_FILE_LOCKING))
		return 0;
#endif
#if defined(_SC_FILE_SYSTEM)
	if(-1 == PyModule_AddIntConstant(module, "SC_FILE_SYSTEM", _SC_FILE_SYSTEM))
		return 0;
#endif
#if defined(_SC_MONOTONIC_CLOCK)
	if(-1 == PyModule_AddIntConstant(module, "SC_MONOTONIC_CLOCK", _SC_MONOTONIC_CLOCK))
		return 0;
#endif
#if defined(_SC_MULTI_PROCESS)
	if(-1 == PyModule_AddIntConstant(module, "SC_MULTI_PROCESS", _SC_MULTI_PROCESS))
		return 0;
#endif
#if defined(_SC_SINGLE_PROCESS)
	if(-1 == PyModule_AddIntConstant(module, "SC_SINGLE_PROCESS", _SC_SINGLE_PROCESS))
		return 0;
#endif
#if defined(_SC_NETWORKING)
	if(-1 == PyModule_AddIntConstant(module, "SC_NETWORKING", _SC_NETWORKING))
		return 0;
#endif
#if defined(_SC_READER_WRITER_LOCKS)
	if(-1 == PyModule_AddIntConstant(module, "SC_READER_WRITER_LOCKS", _SC_READER_WRITER_LOCKS))
		return 0;
#endif
#if defined(_SC_SPIN_LOCKS)
	if(-1 == PyModule_AddIntConstant(module, "SC_SPIN_LOCKS", _SC_SPIN_LOCKS))
		return 0;
#endif
#if defined(_SC_REGEXP)
	if(-1 == PyModule_AddIntConstant(module, "SC_REGEXP", _SC_REGEXP))
		return 0;
#endif
#if defined(_SC_REGEX_VERSION)
	if(-1 == PyModule_AddIntConstant(module, "SC_REGEX_VERSION", _SC_REGEX_VERSION))
		return 0;
#endif
#if defined(_SC_SHELL)
	if(-1 == PyModule_AddIntConstant(module, "SC_SHELL", _SC_SHELL))
		return 0;
#endif
#if defined(_SC_SIGNALS)
	if(-1 == PyModule_AddIntConstant(module, "SC_SIGNALS", _SC_SIGNALS))
		return 0;
#endif
#if defined(_SC_SPAWN)
	if(-1 == PyModule_AddIntConstant(module, "SC_SPAWN", _SC_SPAWN))
		return 0;
#endif
#if defined(_SC_SPORADIC_SERVER)
	if(-1 == PyModule_AddIntConstant(module, "SC_SPORADIC_SERVER", _SC_SPORADIC_SERVER))
		return 0;
#endif
#if defined(_SC_THREAD_SPORADIC_SERVER)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_SPORADIC_SERVER", _SC_THREAD_SPORADIC_SERVER))
		return 0;
#endif
#if defined(_SC_SYSTEM_DATABASE)
	if(-1 == PyModule_AddIntConstant(module, "SC_SYSTEM_DATABASE", _SC_SYSTEM_DATABASE))
		return 0;
#endif
#if defined(_SC_SYSTEM_DATABASE_R)
	if(-1 == PyModule_AddIntConstant(module, "SC_SYSTEM_DATABASE_R", _SC_SYSTEM_DATABASE_R))
		return 0;
#endif
#if defined(_SC_TIMEOUTS)
	if(-1 == PyModule_AddIntConstant(module, "SC_TIMEOUTS", _SC_TIMEOUTS))
		return 0;
#endif
#if defined(_SC_TYPED_MEMORY_OBJECTS)
	if(-1 == PyModule_AddIntConstant(module, "SC_TYPED_MEMORY_OBJECTS", _SC_TYPED_MEMORY_OBJECTS))
		return 0;
#endif
#if defined(_SC_USER_GROUPS)
	if(-1 == PyModule_AddIntConstant(module, "SC_USER_GROUPS", _SC_USER_GROUPS))
		return 0;
#endif
#if defined(_SC_USER_GROUPS_R)
	if(-1 == PyModule_AddIntConstant(module, "SC_USER_GROUPS_R", _SC_USER_GROUPS_R))
		return 0;
#endif
#if defined(_SC_2_PBS)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_PBS", _SC_2_PBS))
		return 0;
#endif
#if defined(_SC_2_PBS_ACCOUNTING)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_PBS_ACCOUNTING", _SC_2_PBS_ACCOUNTING))
		return 0;
#endif
#if defined(_SC_2_PBS_LOCATE)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_PBS_LOCATE", _SC_2_PBS_LOCATE))
		return 0;
#endif
#if defined(_SC_2_PBS_MESSAGE)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_PBS_MESSAGE", _SC_2_PBS_MESSAGE))
		return 0;
#endif
#if defined(_SC_2_PBS_TRACK)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_PBS_TRACK", _SC_2_PBS_TRACK))
		return 0;
#endif
#if defined(_SC_SYMLOOP_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SYMLOOP_MAX", _SC_SYMLOOP_MAX))
		return 0;
#endif
#if defined(_SC_STREAMS)
	if(-1 == PyModule_AddIntConstant(module, "SC_STREAMS", _SC_STREAMS))
		return 0;
#endif
#if defined(_SC_2_PBS_CHECKPOINT)
	if(-1 == PyModule_AddIntConstant(module, "SC_2_PBS_CHECKPOINT", _SC_2_PBS_CHECKPOINT))
		return 0;
#endif
#if defined(_SC_V6_ILP32_OFF32)
	if(-1 == PyModule_AddIntConstant(module, "SC_V6_ILP32_OFF32", _SC_V6_ILP32_OFF32))
		return 0;
#endif
#if defined(_SC_V6_ILP32_OFFBIG)
	if(-1 == PyModule_AddIntConstant(module, "SC_V6_ILP32_OFFBIG", _SC_V6_ILP32_OFFBIG))
		return 0;
#endif
#if defined(_SC_V6_LP64_OFF64)
	if(-1 == PyModule_AddIntConstant(module, "SC_V6_LP64_OFF64", _SC_V6_LP64_OFF64))
		return 0;
#endif
#if defined(_SC_V6_LPBIG_OFFBIG)
	if(-1 == PyModule_AddIntConstant(module, "SC_V6_LPBIG_OFFBIG", _SC_V6_LPBIG_OFFBIG))
		return 0;
#endif
#if defined(_SC_HOST_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_HOST_NAME_MAX", _SC_HOST_NAME_MAX))
		return 0;
#endif
#if defined(_SC_TRACE)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE", _SC_TRACE))
		return 0;
#endif
#if defined(_SC_TRACE_EVENT_FILTER)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_EVENT_FILTER", _SC_TRACE_EVENT_FILTER))
		return 0;
#endif
#if defined(_SC_TRACE_INHERIT)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_INHERIT", _SC_TRACE_INHERIT))
		return 0;
#endif
#if defined(_SC_TRACE_LOG)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_LOG", _SC_TRACE_LOG))
		return 0;
#endif
#if defined(_SC_LEVEL1_ICACHE_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL1_ICACHE_SIZE", _SC_LEVEL1_ICACHE_SIZE))
		return 0;
#endif
#if defined(_SC_LEVEL1_ICACHE_ASSOC)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL1_ICACHE_ASSOC", _SC_LEVEL1_ICACHE_ASSOC))
		return 0;
#endif
#if defined(_SC_LEVEL1_ICACHE_LINESIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL1_ICACHE_LINESIZE", _SC_LEVEL1_ICACHE_LINESIZE))
		return 0;
#endif
#if defined(_SC_LEVEL1_DCACHE_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL1_DCACHE_SIZE", _SC_LEVEL1_DCACHE_SIZE))
		return 0;
#endif
#if defined(_SC_LEVEL1_DCACHE_ASSOC)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL1_DCACHE_ASSOC", _SC_LEVEL1_DCACHE_ASSOC))
		return 0;
#endif
#if defined(_SC_LEVEL1_DCACHE_LINESIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL1_DCACHE_LINESIZE", _SC_LEVEL1_DCACHE_LINESIZE))
		return 0;
#endif
#if defined(_SC_LEVEL2_CACHE_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL2_CACHE_SIZE", _SC_LEVEL2_CACHE_SIZE))
		return 0;
#endif
#if defined(_SC_LEVEL2_CACHE_ASSOC)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL2_CACHE_ASSOC", _SC_LEVEL2_CACHE_ASSOC))
		return 0;
#endif
#if defined(_SC_LEVEL2_CACHE_LINESIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL2_CACHE_LINESIZE", _SC_LEVEL2_CACHE_LINESIZE))
		return 0;
#endif
#if defined(_SC_LEVEL3_CACHE_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL3_CACHE_SIZE", _SC_LEVEL3_CACHE_SIZE))
		return 0;
#endif
#if defined(_SC_LEVEL3_CACHE_ASSOC)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL3_CACHE_ASSOC", _SC_LEVEL3_CACHE_ASSOC))
		return 0;
#endif
#if defined(_SC_LEVEL3_CACHE_LINESIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL3_CACHE_LINESIZE", _SC_LEVEL3_CACHE_LINESIZE))
		return 0;
#endif
#if defined(_SC_LEVEL4_CACHE_SIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL4_CACHE_SIZE", _SC_LEVEL4_CACHE_SIZE))
		return 0;
#endif
#if defined(_SC_LEVEL4_CACHE_ASSOC)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL4_CACHE_ASSOC", _SC_LEVEL4_CACHE_ASSOC))
		return 0;
#endif
#if defined(_SC_LEVEL4_CACHE_LINESIZE)
	if(-1 == PyModule_AddIntConstant(module, "SC_LEVEL4_CACHE_LINESIZE", _SC_LEVEL4_CACHE_LINESIZE))
		return 0;
#endif
#if defined(_SC_IPV6)
	if(-1 == PyModule_AddIntConstant(module, "SC_IPV6", _SC_IPV6))
		return 0;
#endif
#if defined(_SC_RAW_SOCKETS)
	if(-1 == PyModule_AddIntConstant(module, "SC_RAW_SOCKETS", _SC_RAW_SOCKETS))
		return 0;
#endif
#if defined(_SC_V7_ILP32_OFF32)
	if(-1 == PyModule_AddIntConstant(module, "SC_V7_ILP32_OFF32", _SC_V7_ILP32_OFF32))
		return 0;
#endif
#if defined(_SC_V7_ILP32_OFFBIG)
	if(-1 == PyModule_AddIntConstant(module, "SC_V7_ILP32_OFFBIG", _SC_V7_ILP32_OFFBIG))
		return 0;
#endif
#if defined(_SC_V7_LP64_OFF64)
	if(-1 == PyModule_AddIntConstant(module, "SC_V7_LP64_OFF64", _SC_V7_LP64_OFF64))
		return 0;
#endif
#if defined(_SC_V7_LPBIG_OFFBIG)
	if(-1 == PyModule_AddIntConstant(module, "SC_V7_LPBIG_OFFBIG", _SC_V7_LPBIG_OFFBIG))
		return 0;
#endif
#if defined(_SC_SS_REPL_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_SS_REPL_MAX", _SC_SS_REPL_MAX))
		return 0;
#endif
#if defined(_SC_TRACE_EVENT_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_EVENT_NAME_MAX", _SC_TRACE_EVENT_NAME_MAX))
		return 0;
#endif
#if defined(_SC_TRACE_NAME_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_NAME_MAX", _SC_TRACE_NAME_MAX))
		return 0;
#endif
#if defined(_SC_TRACE_SYS_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_SYS_MAX", _SC_TRACE_SYS_MAX))
		return 0;
#endif
#if defined(_SC_TRACE_USER_EVENT_MAX)
	if(-1 == PyModule_AddIntConstant(module, "SC_TRACE_USER_EVENT_MAX", _SC_TRACE_USER_EVENT_MAX))
		return 0;
#endif
#if defined(_SC_XOPEN_STREAMS)
	if(-1 == PyModule_AddIntConstant(module, "SC_XOPEN_STREAMS", _SC_XOPEN_STREAMS))
		return 0;
#endif
#if defined(_SC_THREAD_ROBUST_PRIO_INHERIT)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_ROBUST_PRIO_INHERIT", _SC_THREAD_ROBUST_PRIO_INHERIT))
		return 0;
#endif
#if defined(_SC_THREAD_ROBUST_PRIO_PROTECT)
	if(-1 == PyModule_AddIntConstant(module, "SC_THREAD_ROBUST_PRIO_PROTECT", _SC_THREAD_ROBUST_PRIO_PROTECT))
		return 0;
#endif
	return 1;
}

MODULE_SL_ADD_FUNCTION(pathconf, METH_VARARGS | METH_KEYWORDS);
MODULE_SL_ADD_FUNCTION(sysconf,  METH_VARARGS | METH_KEYWORDS);
MODULE_SL_ADD_INITIALIZER(mod_sys_linux_sysconf_init);
