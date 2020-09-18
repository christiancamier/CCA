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

#ifndef __MOD_SYS_LINUX_SIGNALFD_H__
#define __MOD_SYS_LINUX_SIGNALFD_H__

typedef struct object_signalfd_st {
	PyObject_HEAD
	int       fileno;
	PyObject *sigset;
	sigset_t  sigmsk;
	int       flags;
} object_signalfd;

typedef struct object_signalinfo_st {
	PyObject_HEAD
	struct signalfd_siginfo *signfo;
} object_signalinfo;

#define ATTR_SSI_BAND     1
#define ATTR_SSI_CODE     2
#define ATTR_SSI_ERRNO    3
#define ATTR_SSI_FD       4
#define ATTR_SSI_INT      5
#define ATTR_SSI_OVERRUN  6
#define ATTR_SSI_PID      7
#define ATTR_SSI_SIGNO    8
#define ATTR_SSI_STATUS   9
#define ATTR_SSI_STIME   10
#define ATTR_SSI_TID     11
#define ATTR_SSI_TRAPNO  12
#define ATTR_SSI_UID     13
#define ATTR_SSI_UTIME   14

#endif // ! __MOD_SYS_LINUX_SIGNALFD_H__
