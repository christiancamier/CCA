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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <CCA/display.h>

/* Global functions */

extern void   cc_perror     (const char *);
extern void   cc_printf_err (const char *, ...);
extern void   cc_printf_out (const char *, ...);
extern void   cc_putc_err   (int);
extern void   cc_putc_out   (int);
extern void   cc_puts_err   (const char *);
extern void   cc_puts_out   (const char *);
extern void   cc_vprintf_err(const char *, va_list);
extern void   cc_vprintf_out(const char *, va_list);
extern void (*cc_display_set_err(void (*)(const char *, va_list)))(const char *, va_list);
extern void (*cc_display_set_out(void (*)(const char *, va_list)))(const char *, va_list);

/* Local function */

static void display_err(const char *, va_list); /* Default writer for stderr */
static void display_out(const char *, va_list); /* Default writer for stdout */

static void (*err_displayer)(const char *, va_list) = display_err;
static void (*out_displayer)(const char *, va_list) = display_out;

void cc_perror(const char *message)
{
	int sverr = errno;
	cc_printf_err("%s: %s", message, strerror(sverr));
	errno = sverr;
	return;
}

void cc_printf_err(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	err_displayer(format, ap);
	va_end(ap);
	return;
}

void cc_printf_out(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	out_displayer(format, ap);
	va_end(ap);
	return;
}

void cc_putc_err (int         c) { cc_printf_err("%c", c); return; }
void cc_putc_out (int         c) { cc_printf_out("%c", c); return; }
void cc_puts_err (const char *s) { cc_printf_err("%s", s); return; }
void cc_puts_out (const char *s) { cc_printf_out("%s", s); return; }

void cc_vprintf_err(const char *format, va_list ap)
{
	err_displayer(format, ap);
	return;
}

void cc_vprintf_out(const char *format, va_list ap)
{
	out_displayer(format, ap);
	return;
}

void (*cc_display_set_err(void (*new)(const char *, va_list)))(const char *, va_list)
{
	void (*old)(const char *, va_list) = err_displayer;
	if(new)
		err_displayer = new;
	return old;
}

void (*cc_display_set_out(void (*new)(const char *, va_list)))(const char *, va_list)
{
	void (*old)(const char *, va_list) = out_displayer;
	if(new)
		out_displayer = new;
	return old;
}

static void display_err(const char *fm, va_list ap)
{
	(void)vfprintf(stderr, fm, ap);
	(void)fputc('\n', stderr);
	return;
}

static void display_out(const char *fm, va_list ap)
{
	(void)vfprintf(stdout, fm, ap);
	(void)fputc('\n', stdout);
	return;
}
