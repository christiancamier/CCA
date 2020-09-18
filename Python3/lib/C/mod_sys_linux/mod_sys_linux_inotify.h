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

#ifndef __MOD_SYS_LINUX_INOTIFY_H__
#define __MOD_SYS_LINUX_INOTIFY_H__

typedef struct object_inotify_st {
	PyObject_HEAD
	int       fileno;
} object_inotify;

typedef struct object_inotifyevent_st {
	PyObject_HEAD
	struct inotify_event *event;
} object_inotifyevent;

#define ATTR_IEV_COOKIE	1
#define ATTR_IEV_LEN	2
#define ATTR_IEV_MASK	3
#define ATTR_IEV_NAME	4
#define ATTR_IEV_WD	5

#endif // ! __MOD_SYS_LINUX_INOTIFY_H__
