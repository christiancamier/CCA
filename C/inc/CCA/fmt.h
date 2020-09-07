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
 *	-- CC Utilities: Common format functions
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.0 (01/21/2008)
 */

#ifndef __CC_FMT_H__
#define __CC_FMT_H__

#define cc_fmt_cr(bp, bs)	cc_fmy_char(bp, bs, '\r')
#define cc_fmt_lf(bp, bs)	cc_fmy_char(bp, bs, '\n')

extern       void  cc_fmt_lower_digits(void);
extern       void  cc_fmt_upper_digits(void);
extern const char *cc_fmt_get_digits(void);
extern const char *cc_fmt_get_lower_digits(void);
extern const char *cc_fmt_get_upper_digits(void);

extern ssize_t cc_fmt_bytes  (char **, size_t *, const char *, size_t);
extern ssize_t cc_fmt_char   (char **, size_t *, char);
extern ssize_t cc_fmt_crlf   (char **, size_t *);
extern ssize_t cc_fmt_hexdump(char **, size_t *, const unsigned char *, size_t);
extern ssize_t cc_fmt_int    (char **, size_t *, int,   int);
extern ssize_t cc_fmt_long   (char **, size_t *, long,  int );
extern ssize_t cc_fmt_short  (char **, size_t *, short, int);
extern ssize_t cc_fmt_string (char **, size_t *, const char *);
extern ssize_t cc_fmt_ubytes (char **, size_t *, const unsigned char *, size_t);
extern ssize_t cc_fmt_uchar  (char **, size_t *, unsigned char);
extern ssize_t cc_fmt_uint   (char **, size_t *, unsigned int,   int);
extern ssize_t cc_fmt_ulong  (char **, size_t *, unsigned long,  int );
extern ssize_t cc_fmt_ushort (char **, size_t *, unsigned short, int);

#if defined(INT8_MIN)
extern ssize_t cc_fmt_sint08(char **, size_t *, int8_t,   int);
extern ssize_t cc_fmt_uint08(char **, size_t *, uint8_t,  int);
extern ssize_t cc_fmt_sint16(char **, size_t *, int16_t,  int);
extern ssize_t cc_fmt_uint16(char **, size_t *, uint16_t, int);
extern ssize_t cc_fmt_sint32(char **, size_t *, int32_t,  int);
extern ssize_t cc_fmt_uint32(char **, size_t *, uint32_t, int);
#if defined(INT64_MAX)
extern ssize_t cc_fmt_sint64(char **, size_t *, int64_t,  int);
extern ssize_t cc_fmt_uint64(char **, size_t *, uint64_t, int);
#endif /* INT64_MAX */
#endif /* INT8_MIN */

#endif /*! __CC_FMT_H__*/
