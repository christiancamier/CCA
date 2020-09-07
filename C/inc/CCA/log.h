/*
 * Copyright (c) 2020
 *     Christian CAMIER <christian.c at promethee dot services>
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

#ifndef __DT__LOG_H__
#define __DT__LOG_H__

extern void   cc_log_alert          (const char *, ...);
extern void   cc_log_close          (void);
extern int    cc_log_config         (const char *, const char *);
extern void   cc_log_crit           (const char *, ...);
extern void   cc_log_debug          (const char *, ...);
extern void   cc_log_emerg          (const char *, ...);
extern void   cc_log_err            (const char *, ...);
extern size_t cc_log_format_message (char *, size_t, int, const char *, va_list);
extern void   cc_log_info           (const char *, ...);
extern void   cc_log_notice         (const char *, ...);
extern void   cc_log_perror         (const char *);
extern void   cc_log_reinit         (void);
extern void   cc_log_start          (void);
extern void   cc_log_warning        (const char *, ...);

#ifdef _STDARG_H
extern void cc_log_valert       (const char *, va_list);
extern void cc_log_vcrit        (const char *, va_list);
extern void cc_log_vdebug       (const char *, va_list);
extern void cc_log_vemerg       (const char *, va_list);
extern void cc_log_verr         (const char *, va_list);
extern void cc_log_vinfo        (const char *, va_list);
extern void cc_log_vnotice      (const char *, va_list);
extern void cc_log_vwarning     (const char *, va_list);
#endif

#endif  /* !__CC__LOG_H__ */
