/*
 * Copyright (c) 2008
 *      Christian CAMIER <chcamier@free.fr>
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
#include <ctype.h>
#include <stdarg.h>
#include <CCA/debug.h>
#include <CCA/display.h>
#include <CCA/memory.h>

#include "fmt_internal.h"

extern int    cc_dbg_get_flag(void);
extern void   cc_dbg_memdump(const char *, void *, size_t);
extern void   cc_dbg_printf(const char *, ...);
extern int    cc_dbg_set_flag(int);
extern void (*cc_dbg_set_display(void (*)(const char *, va_list)))(const char *, va_list);
extern void   cc_dbg_vprintf(const char *, va_list);

static int    dbg_flag = 0;
static void (*dbg_displayer)(const char *, va_list) = cc_vprintf_err;

int cc_dbg_get_flag(void) { return dbg_flag; }

void cc_debug_memdump(const char *title, void *address, size_t length)
{
	if(dbg_flag)
	{
		char           l[60];
		int            i;
		int            j;
		unsigned char *p;
		char          *b;
		char          *c;

		cc_dbg_printf("DEBUG: Dump: '%s'\n       Address = %p, lenght = %lu\n", title, address, (unsigned long)length);
		l[54] = '\0';
		for(p = (unsigned char *)address, i = 0; length; i += 16)
		{
			b = l;
			c = l + 40;
			for(j = 0; j < 16 && length; j += 1, p += 1, length -= 1)
			{
				*(b++) = __cc_fmt_digits[((*p) >> 4) & 0x0F];
				*(b++) = __cc_fmt_digits[((*p) >> 0) & 0x0F];
				if(1 == (j & 1))
					*(b++) = ' ';
				*(c++) = isprint(*p) ? *p : '.';
			}
			cc_dbg_printf("       +%04X: %s\n", i, l);
		}
		cc_memclr(l, sizeof(l));
	}
	return;
}

void cc_dbg_printf(const char *format, ...)
{
	if(dbg_flag)
	{
		va_list ap;
		va_start(ap, format);
		dbg_displayer(format, ap);
		va_end(ap);
	}
	return;
}

int cc_dbg_set_flag(int new)
{
	int old = dbg_flag;
	dbg_flag = new;
	return old;
}

void (*cc_dbg_set_display(void (*new)(const char *, va_list)))(const char *, va_list)
{
	void (*old)(const char *, va_list) = dbg_displayer;
	dbg_displayer = new;
	return old;
}

void cc_dbg_vprintf(const char *format, va_list ap)
{
	if(dbg_flag)
		dbg_displayer(format, ap);
	return;
}
