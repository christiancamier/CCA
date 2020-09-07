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
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "erase_internal.h"
#include <CCA/memory.h>

extern erase_status_t _erase_memdup	   (const void *, size_t, void **);
extern erase_status_t _erase_memory_alloc  (size_t, void **);
extern erase_status_t _erase_memory_free   (void *);
extern erase_status_t _erase_memory_realloc(void *, size_t, void **);
extern erase_status_t _erase_strdup        (const char *, char **);

erase_status_t _erase_memdup(const void *src, size_t nby, void **retval)
{
	void *p;

	erase_debug("_erase_memdup(%p, %lu, %p)", src, nby, retval);
	if(CC_TNULL(void) == (p = cc_memdup(src, nby)))
		return ERA_ST_SYSTEM_ERROR;
	*retval = p;
	return ERA_ST_OK;
}

erase_status_t _erase_memory_alloc(size_t nbytes, void **retval)
{
	void *p;

	erase_debug("_erase_memory_alloc(%lu, %p)", nbytes, retval);
	if(CC_TNULL(void) == (p = cc_malloc(nbytes)))
		return ERA_ST_SYSTEM_ERROR;

	*retval = (void *)p;
	return ERA_ST_OK;
}

erase_status_t _erase_memory_realloc(void *pointer, size_t nbytes, void **retval)
{
	void *p;

	erase_debug("_erase_memory_realloc(%p, %lu, %p)", pointer, nbytes, retval);
	if(CC_TNULL(void) == (p = cc_realloc(pointer, nbytes)))
		return ERA_ST_SYSTEM_ERROR;

	*retval = (void *)p;
	return ERA_ST_OK;
}

erase_status_t _erase_memory_free(void *pointer)
{
	erase_debug("_erase_memory_free(%p)", pointer);
	cc_free(pointer);
	return ERA_ST_OK;
}

erase_status_t _erase_strdup(const char *string, char **retval)
{

	char *p;

	erase_debug("_erase_strdup(%p, %p)", string, retval);
	if(CC_TNULL(char) == (p = cc_strdup(string)))

		return ERA_ST_SYSTEM_ERROR;
	*retval = p;
	return ERA_ST_OK;
}
