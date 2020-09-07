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

#include "erase_internal.h"

extern void            _erase_driver_register(erase_driver_t *);
extern erase_status_t  _erase_driver_find(erase_file_t *);

static erase_driver_t *drivers = CC_TNULL(erase_driver_t);

void _erase_driver_register(erase_driver_t *driver)
{
	driver->next = drivers;
	drivers = driver;
	return;
}

erase_status_t _erase_driver_find(erase_file_t *file)
{
	erase_driver_t *drv;
	erase_debug("_erase_driver_find(%p)", file);
	for(drv = drivers; CC_TNULL(erase_driver_t) != drv; drv = drv->next)
	{
		switch(drv->is_a(file))
		{
		case ERA_ST_OK:
			return ERA_ST_OK;
		case ERA_ST_SYSTEM_ERROR:
			return ERA_ST_SYSTEM_ERROR;
		case ERA_ST_ITER_STOP:
		case ERA_ST_DONT_MATCH:
		case ERA_ST_UNKNOWN_METHOD:
		case ERA_ST_METHOD_EXISTS:
		case ERA_ST_METHOD_INCOMPLETE:
		case ERA_ST_NOT_IMPLEMENTED:
		case ERA_ST_CYCLE_DETECTED:
		case ERA_ST_INTERNAL:
		default:
			break;
		}
	}
	return ERA_ST_DONT_MATCH;
}
