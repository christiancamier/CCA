/*
 * Copyright (c) 2010
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

#include <stdarg.h>
#include <stdio.h>

#include "erase_internal.h"

extern void erase_debug(const char *format, ...);

void erase_info(const char *format, ...)
{
	va_list ap;
	if(0 != (_erase_options & (OPTION_VERBOSE | OPTION_DEBUG)))
	{
		va_start(ap, format);
		(void)vsnprintf(_erase_io_buffer, _erase_io_bufsiz, format, ap);
		va_end(ap);
		_erase_cb_info(_erase_io_buffer);
		cc_memclr(_erase_io_buffer, _erase_io_bufsiz);
	}
	return;
}
