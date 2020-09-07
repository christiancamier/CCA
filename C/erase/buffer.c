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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "erase_internal.h"

extern erase_status_t _erase_buffer_create(size_t, erase_buffer_t **);
extern erase_status_t _erase_buffer_destroy(erase_buffer_t *);
extern erase_status_t _erase_fill_buffer(erase_buffer_t *, erase_pass_t *);
extern erase_status_t _erase_fill_buffer_reset(erase_buffer_t *);

static erase_status_t fill_bit_lrotate(erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_bit_rrotate(erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_byte       (erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_byte_rotate(erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_pattern    (erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_random     (erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_random_byte(erase_buffer_t *, erase_pass_t *);
static erase_status_t fill_revert     (erase_buffer_t *, erase_pass_t *);

static erase_status_t (*fillers[])(erase_buffer_t *, erase_pass_t *) = {
	fill_byte,
	fill_random_byte,
	fill_random,
	fill_pattern,
	fill_revert,
	fill_bit_lrotate,
	fill_bit_rrotate,
	fill_byte_rotate,
	fill_byte_rotate
};

erase_status_t fill_bit_lrotate(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long accu;
	unsigned long lshift;
	unsigned long rshift;
	unsigned long wshift;
	unsigned long wbsize;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & ERA_BUF_INITIALIZED))
		return ERA_ST_OK;

	accu   = (unsigned long)ERASE_MODE_ARG(pass->ep_mode);
	wbsize = (unsigned long)BUFFER_ROUNDUP(buffer->bu_size);
	lshift = accu & (ULONG_BITS - 1);
	rshift = ULONG_BITS - lshift;
	wshift = (accu / ULONG_BITS) % wbsize;

	if(wshift)
	{
		unsigned long  b[wshift];
		size_t         i;
		unsigned long *s;
		unsigned long *d;
		for(i = 0, s = buffer->bu_ldata, d = b; i < wshift; i += 1, *(d++) = *(s++));
		for(d = buffer->bu_ldata; i < wbsize; i += 1, *(d++) = *(s++));
		for(i = 0, s = b; i < wshift; i += 1, *(d++) = *(s++));
	}

	if(lshift)
	{
		unsigned long  rrest;
		unsigned long  lmask;
		unsigned long  rmask;
		unsigned long *p;
		size_t         i;
		accu  = 0;
		lmask = ((1UL << lshift) - 1UL) << (ULONG_BITS - lshift);
		rmask = ~lmask;
		p = buffer->bu_ldata;
		rrest = (*p & lmask) >> rshift;
		for(i = 1; i < wbsize; i += 1, p += 1)
		{
			*p <<= lshift;
			*p  |= (*(p + 1) & lmask) >> rshift;
		}
		*p <<= lshift;
		*p  |= rrest;
	}

	buffer->bu_flags |= ERA_BUF_INITIALIZED;
	return ERA_ST_OK;
}

erase_status_t fill_bit_rrotate(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long accu;
	unsigned long lshift;
	unsigned long rshift;
	unsigned long wshift;
	unsigned long wbsize;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & ERA_BUF_INITIALIZED))
		return ERA_ST_OK;

	accu   = (unsigned long)ERASE_MODE_ARG(pass->ep_mode);
	wbsize = (unsigned long)BUFFER_ROUNDUP(buffer->bu_size);
	rshift = accu & (ULONG_BITS - 1);
	lshift = ULONG_BITS - rshift;
	wshift = (accu / ULONG_BITS) % wbsize;

	if(wshift)
	{
		unsigned long  b[wshift];
		size_t         i;
		unsigned long *s;
		unsigned long *d;
		for(i = 0, s = buffer->bu_ldata + wbsize, d = b + wshift; i < wshift; i += 1, *(--d) = *(--s));
		for(d = buffer->bu_ldata + wbsize; i < wbsize; i += 1, *(--d) = *(--s));
		for(i = 0, s = b + wshift; i < wbsize; i += 1, *(--d) = *(--s));
	}

	if(rshift)
	{
		unsigned long  rrest;
		unsigned long  lmask;
		unsigned long  rmask;
		unsigned long *p;
		size_t         i;
		accu = 0;
		rmask = (1UL << rshift) - 1;
		lmask = ~rmask;
		p = buffer->bu_ldata + wbsize - 1;
		rrest = (*p & rmask) << lshift;
		for(i = 1; i < wbsize; i += 1, p -= 1)
		{
			*p >>= rshift;
			*p  |= (*(p - 1) & rmask) << lshift;
		}
		*p >>= rshift;
		*p  |= rrest;
	}

	buffer->bu_flags |= ERA_BUF_INITIALIZED;
	return ERA_ST_OK;
}

erase_status_t fill_byte(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long  fval;
	unsigned long *bpos;
	size_t         bfil;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & ERA_BUF_INITIALIZED))
		return ERA_ST_OK;

	fval = (unsigned long)ERASE_MODE_ARG(pass->ep_mode);
	fval = (fval <<  8) | fval;
	fval = (fval << 16) | fval;
#ifdef ARCH_64BITS
	fval = (fval << 32) | fval;
#endif
	for(bpos = buffer->bu_ldata, bfil = BUFFER_ROUNDUP(buffer->bu_size); bfil > 0; *(bpos++) = fval, bfil -= 1);
	buffer->bu_flags |= ERA_BUF_INITIALIZED;
	return ERA_ST_OK;
}

erase_status_t fill_byte_rotate(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long code;
	size_t        nrot;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & ERA_BUF_INITIALIZED))
		return ERA_ST_OK;

	code = ERASE_MODE_CODE(pass->ep_mode);
	nrot = (((size_t)ERASE_MODE_ARG(pass->ep_mode)) % buffer->bu_size);

	if(nrot != 0)
	{
		unsigned char  buf[nrot];
		unsigned char *dst;
		size_t         cnt;
		int            inc;
		unsigned char *ret;
		size_t         smv;
		unsigned char *src;
		smv = buffer->bu_size - nrot;
		if(ERASE_MODE_BIT_LROTATE_CODE == code)
		{
			dst = buffer->bu_cdata + buffer->bu_size - 1;
			inc = -1;
			ret = buffer->bu_cdata;
			src = buffer->bu_cdata + smv - 1;
		}
		else
		{
			dst = buffer->bu_cdata;
			inc = 1;
			ret = buffer->bu_cdata + smv - 1;
			src = buffer->bu_cdata + nrot;
		}
		(void)memcpy((void *)buf, (void *)src, nrot);
		for(cnt = 0; cnt < smv; cnt += 1, *dst = *src, dst += inc, src += inc);
		(void)memcpy((void *)ret, (void *)buf, nrot);
	}

	buffer->bu_flags |= ERA_BUF_INITIALIZED;
	return ERA_ST_OK;
}

erase_status_t fill_pattern(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned char *bpos;
	size_t         bfil;
	unsigned char *wsrc;
	size_t         wlen;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & ERA_BUF_INITIALIZED))
		return ERA_ST_OK;

	bpos = buffer->bu_cdata;
	bfil = buffer->bu_size;

	while(bfil > 0)
	{
		wsrc = (unsigned char *)pass->ep_pattern_data;
		for(wlen = CC_MIN(bfil, pass->ep_pattern_size); wlen > 0; wlen -= 1, bfil -= 1, *(bpos++) = *(wsrc++));
	}

	buffer->bu_flags |= ERA_BUF_INITIALIZED;
	return ERA_ST_OK;
}

erase_status_t fill_random(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long *bpos;
	size_t         bfil;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & (ERA_BUF_INITIALIZED | ERA_BUF_RECOMPUTE)))
		return ERA_ST_OK;

	for(bpos = buffer->bu_ldata, bfil = BUFFER_ROUNDUP(buffer->bu_size); bfil > 0; *(bpos++) = _erase_random(), bfil -= 1);

	if(ERA_BUF_INITIALIZED != (buffer->bu_flags & ERA_BUF_INITIALIZED))
	{
		buffer->bu_flags |= ERA_BUF_INITIALIZED;
		if(ERASE_MODE_ARG(pass->ep_mode))
			buffer->bu_flags |= ERA_BUF_RECOMPUTE;
	}
	return ERA_ST_OK;
}

erase_status_t fill_random_byte(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long  fval;
	unsigned long *bpos;
	size_t         bfil;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & (ERA_BUF_INITIALIZED | ERA_BUF_RECOMPUTE)))
		return ERA_ST_OK;

	fval = _erase_random() & 0xFF;
	fval = (fval <<  8) | fval;
	fval = (fval << 16) | fval;
#ifdef ARCH_64BITS
	fval = (fval << 32) | fval;
#endif

	for(bpos = buffer->bu_ldata, bfil = BUFFER_ROUNDUP(buffer->bu_size); bfil > 0; *(bpos++) = fval, bfil -= 1);

	if(ERA_BUF_INITIALIZED != (buffer->bu_flags & ERA_BUF_INITIALIZED))
	{
		buffer->bu_flags |= ERA_BUF_INITIALIZED;
		if(ERASE_MODE_ARG(pass->ep_mode))
			buffer->bu_flags |= ERA_BUF_RECOMPUTE;
	}
	return ERA_ST_OK;
}

erase_status_t fill_revert(erase_buffer_t *buffer, erase_pass_t *pass)
{
	unsigned long *bpos;
	size_t         bfil;

	(void)pass;

	if(ERA_BUF_INITIALIZED == (buffer->bu_flags & ERA_BUF_INITIALIZED))
		return ERA_ST_OK;

	for(bpos = buffer->bu_ldata, bfil = BUFFER_ROUNDUP(buffer->bu_size); bfil > 0; bfil -= 1, *(bpos++) ^= ~0UL);
	buffer->bu_flags |= ERA_BUF_INITIALIZED;
	return ERA_ST_OK;
}

erase_status_t _erase_buffer_create(size_t size, erase_buffer_t **buffer)
{
	size_t          bs;
	size_t          ts;
	unsigned long  *bm;
	erase_buffer_t *bu;
	erase_status_t  st;

	erase_debug("_erase_buffer_create(%lu, %p)", size, buffer);
	bs = BUFFER_BYTES(BUFFER_ROUNDUP(size));
	ts = bs + BUFFER_HDRSIZE;

	if(ERA_ST_OK == (st = _erase_memory_alloc(ts, (void **)&bm)))
	{
		bu  = (erase_buffer_t *)bm;
		bu->bu_flags = 0;
		bu->bu_size  = sizeof(unsigned long) * bs;
		*buffer = bu;
	}
	return st;
}

erase_status_t _erase_buffer_destroy(erase_buffer_t *buffer)
{
	erase_debug("_erase_buffer_destroy(%p)", buffer);
	return _erase_memory_free(buffer);
}

erase_status_t _erase_fill_buffer_reset(erase_buffer_t *buffer)
{
	erase_debug("_erase_buffer_reset(%p)", buffer);
	buffer->bu_flags = 0;
	return ERA_ST_OK;
}

erase_status_t _erase_fill_buffer(erase_buffer_t *buffer, erase_pass_t *pass)
{
#define filler_max ((sizeof(fillers) / sizeof(void (*)(erase_buffer_t *, erase_pass_t *))) - 1)
	size_t code = (int)ERASE_MODE_CODE(pass->ep_mode);
	erase_debug("_erase_buffer_fill_buffer(%p, %p)", buffer, pass);
	if(code > filler_max || code < 0)
	{
		fprintf(stderr, "Internal error: Bad fill function code %02X\n", (int)code);
		fprintf(stderr, "File: %s, Function: _erase_fill_buffer\n", __FILE__);
		return ERA_ST_INTERNAL;
	}
	return fillers[code](buffer, pass);
}
