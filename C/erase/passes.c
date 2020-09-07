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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "erase_internal.h"

extern erase_status_t _erase_pass_process(erase_file_t *, erase_method_t *, uint64_t);

extern erase_status_t  erase_pass_create_bit_lrotate(unsigned long, erase_pass_t **);
extern erase_status_t  erase_pass_create_bit_rrotate(unsigned long, erase_pass_t **);
extern erase_status_t  erase_pass_create_byte(unsigned long, erase_pass_t **);
extern erase_status_t  erase_pass_create_byte_lrotate(unsigned long, erase_pass_t **);
extern erase_status_t  erase_pass_create_byte_rrotate(unsigned long, erase_pass_t **);
extern erase_status_t  erase_pass_create_invert(erase_pass_t **);
extern erase_status_t  erase_pass_create_method(erase_method_t *, erase_pass_t **);
extern erase_status_t  erase_pass_create_method_name(const char *, erase_pass_t **);
extern erase_status_t  erase_pass_create_pattern(unsigned char *, size_t, erase_pass_t **);
extern erase_status_t  erase_pass_create_random(int, erase_pass_t **);
extern erase_status_t  erase_pass_create_rndbyte(int, erase_pass_t **);
extern erase_status_t  erase_pass_destroy(erase_pass_t *);

static erase_status_t builder(unsigned long, unsigned long, erase_pass_t **);

static erase_status_t builder(unsigned long code, unsigned long arg, erase_pass_t **pass)
{
	erase_status_t  status;

	erase_debug("builder(%02lX, %02lX, %p)", code, arg, pass);
	if(ERA_ST_OK == (status = _erase_memory_alloc(sizeof(erase_pass_t), (void **)pass)))
		(*pass)->ep_mode = ERASE_MAKE_MODE(code, arg);
	return status;
}

erase_status_t erase_pass_create_bit_lrotate(unsigned long nrot, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_bit_lrotate(%lu, %p)", nrot, pass);
	return builder(ERASE_MODE_BIT_LROTATE_CODE, nrot, pass);
}

erase_status_t erase_pass_create_bit_rrotate(unsigned long nrot, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_bit_rrotate(%lu, %p)", nrot, pass);
	return builder(ERASE_MODE_BIT_RROTATE_CODE, nrot, pass);
}

erase_status_t erase_pass_create_byte(unsigned long byte, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_byte(%lu, %p)", byte, pass);
	return builder(ERASE_MODE_BYTE_CODE, byte, pass);
}

erase_status_t erase_pass_create_byte_lrotate(unsigned long nrot, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_byte_lrotate(%lu, %p)", nrot, pass);
	return builder(ERASE_MODE_BYTE_LROTATE_CODE, nrot, pass);
}

erase_status_t erase_pass_create_byte_rrotate(unsigned long nrot, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_byte_rrotate(%lu, %p)", nrot, pass);
	return builder(ERASE_MODE_BYTE_RROTATE_CODE, nrot, pass);
}

erase_status_t erase_pass_create_invert(erase_pass_t **pass)
{
	erase_debug("erase_pass_create_invert(%p)", pass);
	return builder(ERASE_MODE_INVERT_CODE, 0, pass);
}

erase_status_t erase_pass_create_method(erase_method_t *method, erase_pass_t **pass)
{
	erase_status_t  status;

	erase_debug("erase_pass_create_method(%p, %p)", method, pass);
	if(ERA_ST_OK == (status = builder(ERASE_MODE_METHOD_CODE, 0xFF, pass)))
	{
		(*pass)->ep_method_def = method;
		method->em_refcnt += 1;
	}
	return status;
}

erase_status_t erase_pass_create_method_name(const char *method, erase_pass_t **pass)
{
	erase_status_t  status;
	char           *name;
	erase_method_t *meth;

	erase_debug("erase_pass_create_method_name(%s, %p)", method, pass);
	if(ERA_ST_OK == (status = erase_method_find(method, &meth)))
		return erase_pass_create_method(meth, pass);
	if(OPTION_BACK_NAME_RESOLUTION != (_erase_options & OPTION_BACK_NAME_RESOLUTION))
		return ERA_ST_UNKNOWN_METHOD;
	if(ERA_ST_OK != (status = _erase_strdup(method, &name)))
		return status;
	if(ERA_ST_OK == (status = builder(ERASE_MODE_METHOD_CODE, 0x00, pass)))
		(*pass)->ep_method_name = name;
	else
		CC_PROTECT_ERRNO(_erase_memory_free(name));
	return status;
}

erase_status_t erase_pass_create_pattern(unsigned char *pattern, size_t patsize, erase_pass_t **pass)
{
	erase_status_t  status;
	unsigned char  *patt;

	erase_debug("erase_pass_create_pattern(%p, %lu, %p)", pattern, patsize, pass);
	if(ERA_ST_OK != (status = _erase_memdup(pattern, patsize, (void **)&patt)))
		return status;
	if(ERA_ST_OK == (status = builder(ERASE_MODE_PATTERN_CODE, 0x00, pass)))
	{
		(*pass)->ep_pattern_data = patt;
		(*pass)->ep_pattern_size = patsize;
	}
	else
	{
		CC_PROTECT_ERRNO(_erase_memory_free(patt));
	}
	return ERA_ST_OK;
}

erase_status_t erase_pass_create_random(int each, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_random(%d, %p)", each, pass);
	return builder(ERASE_MODE_RANDOM_CODE, each ? 0xFF : 0x00, pass);
}

erase_status_t erase_pass_create_rndbyte(int each, erase_pass_t **pass)
{
	erase_debug("erase_pass_create_rndbyte(%d, %p)", each, pass);
	return builder(ERASE_MODE_RNDBYTE_CODE, each ? 0xFF : 0x00, pass);
}

erase_status_t erase_pass_destroy(erase_pass_t *pass)
{
	erase_debug("erase_pass_destroy(%p)", pass);

	if(ERASE_REFCNT_STATIC == pass->ep_refcnt)
	{
		erase_debug(" . Pass %p is static", pass);
		return ERA_ST_IS_STATIC;
	}
	if(pass->ep_refcnt)
		pass->ep_refcnt -= 1;
	if(0 < pass->ep_refcnt)
	{
		erase_debug(" . Reference counter ajusted to %lu", pass->ep_refcnt);
		return ERA_ST_OK;
	}
	switch(ERASE_MODE_CODE(pass->ep_mode))
	{
	case ERASE_MODE_PATTERN_CODE:
		_erase_memory_free(pass->ep_pattern_data);
		break;
	case ERASE_MODE_METHOD_CODE:
		if(0 == ERASE_MODE_ARG(pass->ep_mode))
			(void)_erase_memory_free(pass->ep_method_name);
		else
			(void)erase_method_destroy(pass->ep_method_def);
		break;
	case ERASE_MODE_BIT_LROTATE_CODE:
	case ERASE_MODE_BIT_RROTATE_CODE:
	case ERASE_MODE_BYTE_CODE:
	case ERASE_MODE_BYTE_LROTATE_CODE:
	case ERASE_MODE_BYTE_RROTATE_CODE:
	case ERASE_MODE_INVERT_CODE:
	case ERASE_MODE_RANDOM_CODE:
	case ERASE_MODE_RNDBYTE_CODE:
	default:
		break;
	}
	erase_debug(" . method destroyed");
	return _erase_memory_free(pass);
}

erase_status_t _erase_pass_process(erase_file_t *file, erase_method_t *method, uint64_t passno)
{
	erase_status_t  status;
	erase_pass_t   *pass;
	pass = *(method->em_passes + passno);
	_erase_cb_pass_start(file->fname, method, passno);
	if(ERASE_MODE_METHOD_CODE == ERASE_MODE_CODE(pass->ep_mode))
		status = _erase_method_process(file, pass->ep_method_def);
	else
		status = file->driver->process_pass(file, method, passno);
	if(ERA_ST_OK != status)
		return status;
	_erase_cb_pass_end(file->fname, method, passno);
	return ERA_ST_OK;
}
