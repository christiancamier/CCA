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
 * #@ "cc_fmt_uint64.c"
 *	-- CC Utilities: Unsigned integer buffer formating
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (01/22/2008)
 */
#include <stdint.h>

#if HAVE_UINT64_T
#define __TYP uint64_t
#define __FCT cc_fmt_uint64
#include "fmt_uintXX.h"
#undef __FCT
#undef __TYP
#endif
