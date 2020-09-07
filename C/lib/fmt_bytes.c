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

/*
 * #@ "cc_fmt_bytes.c"
 *	-- CC Utilities: Byte buffer formating
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (01/22/2008)
 */

#include <sys/types.h>
#include <errno.h>

#include "fmt_internal.h"

extern ssize_t cc_fmt_bytes(char **, size_t *, const char *, size_t);

ssize_t cc_fmt_bytes(char **buffer, size_t *bufsiz, const char *value, size_t valsz)
{
	char       *p =  (char *)*buffer;
	const char *v =  value;
	size_t      s = *bufsiz;
	size_t      l =  valsz;
	size_t      r =  valsz;

	errno = 0;

	if(s < l)
	{
		errno = ENOSPC;
		l = r = s;
	}

	while(l--)
		*(p++) = *(v++);

	*buffer += r;
	*bufsiz -= r;
	return (ssize_t)r;
}
