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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include <CCA/util.h>
#include <CCA/memory.h>

extern void *cc_calloc(size_t, size_t);
extern void  cc_free(void *);
extern void *cc_malloc(size_t);
extern void  cc_memclr(void *, size_t);
extern void *cc_memdup(const void *, size_t);
extern void *cc_realloc(void *, size_t);
extern char *cc_strdup(const char *);

#define ULONG_SIZE(nbytes)      (((unsigned long)(nbytes) + sizeof(unsigned long) + sizeof(unsigned long) - 1) / sizeof(unsigned long))
#define ULONG2BYTES(nulong)     ((unsigned long)(nulong) * sizeof(unsigned long))

void *cc_calloc(size_t nmemb, size_t esize)
{
	return cc_malloc(nmemb * esize);
}

extern void  cc_free(void *ptr)
{
	unsigned long  i;
	unsigned long  n;
	unsigned long *p;
	unsigned long *s;

	s  = (unsigned long *)ptr;
	s -= 1;
	n  = *s;

	for(i = 0, p = s; i < n; *(p++) =   0UL, i += 1);
	free(s);
	return;
}

extern void *cc_malloc(size_t size)
{
	unsigned long  i;
	unsigned long  n;
	unsigned long *p;
	unsigned long *r;

	n = ULONG_SIZE(size);
	if(CC_TNULL(unsigned long) == (r = (unsigned long *)malloc(ULONG2BYTES(n))))
		return CC_TNULL(void);

	*(r++) = n;
	for(p = r, i = 1; i < n; *(p++) = 0UL, i += 1);
	return (void *)r;
}

extern void  cc_memclr(void *ptr, size_t size)
{
	(void)memset(ptr, 0x00, size); /* fall into */
	return;
}

extern void *cc_memdup(const void *address, size_t size)
{
	void *dest;

	if(CC_TNULL(void) != (dest = cc_malloc(size)))
		(void)memcpy(dest, address, size);

	return dest;
}

extern void *cc_realloc(void *ptr, size_t newsize)
{
	unsigned long  i;
	unsigned long  n;
	unsigned long  o;
	unsigned long *p;
	unsigned long *s;

	p = (unsigned long *)ptr;
	s = p - 1;
	o = *s;
	n = ULONG_SIZE(newsize);
	if(n < o)
		return ptr;
	if(CC_TNULL(unsigned long) == (s = p = (unsigned long *)realloc(s, ULONG2BYTES(n))))
		return CC_TNULL(void);
	*(s++) = n;
	for(i = o, p += o; i < n; *(p++) = 0UL, i += 1);
	return (void *)s;
}

extern char *cc_strdup(const char *string)
{
	return (char *)cc_memdup((void *)string, strlen(string) + 1);
}
