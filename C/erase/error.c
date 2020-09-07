/*-
 * Copyright (c) 2010
 * 	Christian CAMIER <chcamier@free.fr>
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
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "erase_internal.h"

extern void        erase_print_error(erase_status_t, const char *format, ...);
extern const char *erase_error_string(erase_status_t);

const char *erase_error_string(erase_status_t status)
{
	switch(status)
	{
	case ERA_ST_CYCLE_DETECTED:		return "cycle detected";
	case ERA_ST_DONT_MATCH:			return "does not match";
	case ERA_ST_END_OF_FILE:		return "end of file";
	case ERA_ST_INTERNAL:			return "internal error";
	case ERA_ST_IS_STATIC:			return "is static";
	case ERA_ST_ITER_STOP:			return "end of iteration";
	case ERA_ST_LINE_TOO_LONG:		return "line too long";
	case ERA_ST_METHOD_EXISTS:		return "method exists";
	case ERA_ST_METHOD_INCOMPLETE:		return "method is incomplete";
	case ERA_ST_NOT_IMPLEMENTED:		return "not implemented";
	case ERA_ST_OK:				return "ok";
	case ERA_ST_PREMATURE_END_OF_FILE:	return "prematured end of file";
	case ERA_ST_SYNTAX_ERROR:		return "syntax error";
	case ERA_ST_SYSTEM_ERROR:		return strerror(errno);
	case ERA_ST_UNKNOWN_METHOD:		return "method does not exists";
	case ERA_ST_VALUE_ERROR:		return "value error";
	case ERA_ST_WARNING:			return "warning";
	}
	return "unknown error";
}

void erase_print_error(erase_status_t status, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	CC_PROTECT_ERRNO(vsnprintf(_erase_io_buffer, _erase_io_bufsiz, format, ap));
	va_end(ap);
	CC_PROTECT_ERRNO(
		fprintf(stderr, "%s: %s\n", _erase_io_buffer, erase_error_string(status));
		cc_memclr(_erase_io_buffer, _erase_io_bufsiz));
	return;
}
