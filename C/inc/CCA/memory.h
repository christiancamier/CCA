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

/*
 * #@ "cc_memory.h"
 *	-- CC Utilities: Memory management utilities
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (02/21/2009)
 */

#ifndef __CC_MEMORY_H__
#define __CC_MEMORY_H__

/*
 * CC_TALLOC:
 * ----------
 *
 * type T *CC_TALLOC(type T, size_t N)
 *
 * Allocate N elements of type T
 */

#define CC_TALLOC(T,N)	     (T *)cc_malloc(N * sizeof(T))
#define CC_TREALLOC(P, T, N) (P = (T*)cc_realloc(P, N * sizeof(T)))
/*
 * CC_ALLOCA:
 * ----------
 *
 * void *CC_ALLOCA(size_t size)
 * void  CC_FREEA(void *pointer)
 *
 * alloca substitution if not implemented
 *
 */
#ifdef HAVE_ALLOCA
# define CC_ALLOCA(S)    alloca(S)
# define CC_FREEA(P, S)  cc_memclr(P, S)
#else
# define CC_ALLOCA(S)    cc_malloc(S)
# define CC_FREEA(P, S)  cc_free(P)
#endif

/*
 * CC_TALLOCA:
 * -----------
 *
 * type T *CC_TALLOCA(type T, size_t N)
 *
 * Allocate N elements of type T using alloca.
 */
#define CC_TALLOCA(T, N) (T *)CC_ALLOCA(N * sizeof(T))

/*
 * Functions defined in cc_memory.c
 */
extern void *cc_calloc(size_t, size_t);
extern void  cc_free(void *);
extern void *cc_malloc(size_t);
extern void  cc_memclr(void *, size_t);
extern void *cc_memdup(const void *, size_t);
extern void *cc_realloc(void *, size_t);
extern char *cc_strdup(const char *);

#endif /*!__CC_MEMORY_H__*/
