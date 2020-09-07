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

#ifndef __CC_UTIL_H__
#define __CC_UTIL_H__

#define CC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CC_MAX(a, b) ((a) > (b) ? (a) : (b))

#define CC_PROTECT_ERRNO(code)	\
	{			\
		int e = errno;	\
		code;		\
		errno = e;	\
	}

#define CC_STRUCT_OFFSET_OF(T, F) (size_t)(&((T *)0)->F)

/*
 * CC_TNULL:
 * ---------
 *
 * type T *CC_TNULL(type T)
 */
#define CC_TNULL(T)		((T *)NULL)
#define CC_IFNULL(T, V, D)	(CC_TNULL(T) == (V) ? (D) : (V))

#define CC_ARRAY_COUNT(A)	(sizeof(A) / sizeof(A[0]))

#endif /*!__CC_UTIL_H__*/
