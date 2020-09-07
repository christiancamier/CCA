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

#include <stdio.h>
#include <stdlib.h>

#include <CCA/memory.h>

#include "erase_internal.h"

extern void            erase_warnings_walk(erase_cb_msg_t);
extern void           _erase_warning_add(erase_warning_t *);
extern void           _erase_warning_display(void);
extern erase_status_t _erase_warning_exist(void);

erase_warning_t *warnings = CC_TNULL(erase_warning_t);

void _erase_warning_add(erase_warning_t *warning)
{
	warning->next = warnings;
	warnings = warning;
	return;
}

void _erase_warning_display(void)
{
	erase_warning_t *p;
	for(p = warnings; p; p = p->next)
		fprintf(stderr, "WARNING: %s\n", p->message);
	return;
}

erase_status_t _erase_warning_exist(void)
{
	return warnings ? ERA_ST_WARNING : ERA_ST_OK;
}

void erase_warnings_walk(erase_cb_msg_t callback)
{
	erase_warning_t *p;
	for(p = warnings; p; p = p->next)
		callback(p->message);
	return;
}
	
