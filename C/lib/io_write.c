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
 * #@ "cc_io_write.c"
 *	-- CC Utilities: force complete write except if an error occurs
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (01/22/2008)
 */

#include <sys/types.h>
#include <unistd.h>
#include <CCA/io.h>

extern ssize_t cc_io_write(int, const void *, size_t, size_t *, const void **);

ssize_t cc_io_write(int fd, const void *buf, size_t count, size_t *wcnt, const void **endp)
{
	const   char *bufpos;
	ssize_t       wcount;
	size_t        reman;
	for(bufpos = (const char *)buf, wcount = 0, reman = count; reman > 0; reman -= wcount, bufpos += wcount)
	{
		if(-1 == (wcount = write(fd, (const void *)bufpos, reman)))
			break;
	}
	if(wcnt) *wcnt = (size_t)(bufpos - (const char *)buf);
	if(endp) *endp = (const void *)bufpos;
	return -1 == wcount ? -1 : count;
}
