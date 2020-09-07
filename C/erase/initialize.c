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

extern unsigned long _erase_options;
extern char          _erase_io_buffer[];
extern size_t        _erase_io_bufsiz;

unsigned long _erase_options   = 0;
char          _erase_io_buffer[1204];
size_t        _erase_io_bufsiz = sizeof(_erase_io_buffer);

extern erase_status_t erase_initialize(void);
extern int            erase_option_isset_back_name_resolution(void);
extern int            erase_option_isset_debug(void);
extern int            erase_option_isset_simulation(void);
extern int            erase_option_isset_verbose(void);
extern void           erase_option_set_back_name_resolution(int);
extern void           erase_option_set_debug(int);
extern void           erase_option_set_simulation(int);
extern void           erase_option_set_verbose(int);

erase_status_t erase_initialize(void)
{
	erase_debug("erase_initialize()");
	_erase_warning_display();
	return _erase_warning_exist();
}

#define OPTION(name, option)						\
	void erase_option_set_##name(int bool)				\
	{								\
		erase_debug("erase_option_set_" #name "(%d)", bool);	\
		if(bool) _erase_options |= option;			\
		else     _erase_options &= ~option;			\
		return;							\
	}								\
	int erase_option_isset_##name(void)				\
	{								\
		erase_debug("rease_option_isset" #name "()");		\
		return option == (_erase_options & option);		\
	}
OPTION(back_name_resolution, OPTION_BACK_NAME_RESOLUTION);
OPTION(debug,                OPTION_DEBUG)
OPTION(simulation,           OPTION_SIMULATION)
OPTION(verbose,              OPTION_VERBOSE)
#undef SETOPTION
