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

#include <errno.h>
#include <stdlib.h>

#include <CCA/util.h>

#include "erase_internal.h"

extern erase_status_t erase_remove_file(const char *, erase_method_t *);

erase_status_t erase_remove_file(const char *filename, erase_method_t *method)
{
	erase_file_t   *file;
	erase_status_t  status;
	(void)method;
	if(ERA_ST_OK != (status = _erase_file_open(filename, &file)))
		return status;
	_erase_cb_file_start(filename);
	if(ERA_ST_OK != (status = file->driver->initialize(file)))
	{
		CC_PROTECT_ERRNO((void)_erase_file_close(file));
		return status;
	}
	if(ERA_ST_OK != (status = _erase_method_process(file, method)))
	{
		CC_PROTECT_ERRNO((void)_erase_file_close(file));
		return status;
	}
	if(ERA_ST_OK != (status = file->driver->finalize(file)))
	{
		CC_PROTECT_ERRNO((void)_erase_file_close(file));
		return status;
	}
	(void)_erase_file_close(file);
	_erase_cb_file_end(filename);
	return ERA_ST_OK;
}
