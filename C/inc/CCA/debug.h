/*
 * Copyright (c) 2008
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
 * #@ "cc_format.h"
 *	-- CC Utilities: Debug mode functions
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (01/21/2008)
 */

#ifndef __CC_DEBUG_H__
#define __CC_DEBUG_H__

extern int    cc_dbg_get_flag(void);
extern void   cc_dbg_memdump(const char *, void *, size_t);
extern void   cc_dbg_printf(const char *, ...);
extern int    cc_dbg_set_flag(int);
extern void (*cc_dbg_set_display(void (*)(const char *, va_list)))(const char *, va_list);

#ifdef _STDARG_H
extern void   cc_dbg_vprintf(const char *, va_list);
#endif
#endif /*! __CC_DEBUG_H__*/
