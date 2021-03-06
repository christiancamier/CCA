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

#ifndef __MOD_SYS_LINUX_SYSCONF_H__
#define __MOD_SYS_LINUX_SYSCONF_H__

#define ATTR_UPTIME	 1
#define ATTR_LOAD_1	 2
#define ATTR_LOAD_5	 3
#define ATTR_LOAD_15	 4
#define ATTR_LOADS	 5
#define ATTR_TOTALRAM	 6
#define ATTR_FREERAM	 7
#define ATTR_SHAREDRAM	 8
#define ATTR_BUFFERRAM	 9
#define ATTR_TOTALSWAP	10
#define ATTR_FREESWAP	11
#define ATTR_PROCS	12
#define ATTR_TOTALHIGH	13
#define ATTR_FREEHIGH	14
#define ATTR_MEM_UNIT	15

#endif // ! __MOD_SYS_LINUX_SYSCONF_H__
