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
 * #@ "cc_fmt.c"
 *	-- CC Utilities: Global definitions for format functions
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (01/22/2008)
 */

#include <sys/types.h>

#include "fmt_internal.h"

extern       void    cc_fmt_lower_digits(void);
extern       void    cc_fmt_upper_digits(void);
extern const char   *cc_fmt_get_digits(void);
extern const char   *cc_fmt_get_lower_digits(void);
extern const char   *cc_fmt_get_upper_digits(void);
extern       size_t  cc_fmt_base_min(void);
extern       size_t  cc_fmt_base_max(void);


const char    __cc_fmt_digits_u[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
	'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};
const char    __cc_fmt_digits_l[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b',
	'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};
const char         *__cc_fmt_digits  = __cc_fmt_digits_u;
const unsigned int  __cc_fmt_basemin = 2;
const unsigned int  __cc_fmt_basemax = sizeof(__cc_fmt_digits_u);

/*
 * NAME
 *	cc_fmt_lower_digits, cc_fmt_upper_digits - Select digit case
 *
 * SYNOPSIS
 *	#include <CCA/fmt.h>
 *	void cc_fmt_lower_digits(void)
 *	void cc_fmt_upper_digits(void)
 *
 * DESCRIPTION
 *	The function cc_fmt_lower_digits (cc_fmt_upper_digits) select
 *	the lower (upper) case digits as default digit set for the fmt
 *	functions.
 *
 * RETURN VALUE
 *	None
 */
void cc_fmt_lower_digits(void) { __cc_fmt_digits = __cc_fmt_digits_l; return; }
void cc_fmt_upper_digits(void) { __cc_fmt_digits = __cc_fmt_digits_u; return; }

/*
 * NAME
 *	cc_fmt_get_digits, cc_fmt_get_lower_digits, cc_fmt_get_upper_digits
 *
 * SYNOPSIS
 *	#include <CCA/fmt.h>
 *	const char *cc_fmt_get_digits(void);
 *	const char *cc_fmt_get_lower_digits(void);
 *	const char *cc_fmt_get_upper_digits(void);
 *
 * DESCRIPTION
 *	cc_fmt_get_digits returns a pointer to an array of 36 characters
 *	representing the current defaults digits.
 *
 *	cc_fmt_get_lower_digits returns a pointer to an array of 36 characters
 *	representing the lower case digits.
 *
 *	cc_fmt_get_upper_digits returns a pointer to an array of 36 characters
 *	representing the upper case digits.
 *
 * RETURN VALUE
 *	A pointer to a 36 character array.
 *
 * NOTE
 *	The character array is not terminated by a null character.
 */
const char *cc_fmt_get_digits(void)       { return __cc_fmt_digits;   }
const char *cc_fmt_get_lower_digits(void) { return __cc_fmt_digits_l; }
const char *cc_fmt_get_upper_digits(void) { return __cc_fmt_digits_u; }

size_t cc_fmt_base_min(void) { return __cc_fmt_basemin; }
size_t cc_fmt_base_max(void) { return __cc_fmt_basemax; }
