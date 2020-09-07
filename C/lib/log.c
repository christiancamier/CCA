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
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <CCA/io.h>
#include <CCA/memory.h>
#include <CCA/util.h>

#include "log_internal.h"


/* Global functions  */
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
extern void   cc_log_register_driver(struct cc_log_driver_st *);
extern void   cc_log_reinit         (void);
extern void   cc_log_start          (void);
extern void   cc_log_valert         (const char *, va_list);
extern void   cc_log_vcrit          (const char *, va_list);
extern void   cc_log_vdebug         (const char *, va_list);
extern void   cc_log_vemerg         (const char *, va_list);
extern void   cc_log_verr           (const char *, va_list);
extern void   cc_log_vinfo          (const char *, va_list);
extern void   cc_log_vnotice        (const char *, va_list);
extern void   cc_log_vwarning       (const char *, va_list);
extern void   cc_log_warning        (const char *, ...);

extern const struct cc_log_code_st *cc_log_search_name(const char *, const struct cc_log_code_st *, size_t);
extern const struct cc_log_code_st *cc_log_search_code(int,          const struct cc_log_code_st *, size_t);

/* Local functions */
static void        do_log(int, const char *, va_list);

/* Local variables */
static const char    tfmt_def[] = default_tfm;
static int           loglevel   = default_lvl;
static const char   *timefmt    = tfmt_def;

static struct cc_log_driver_st *drivers = NULL; /* Defined drivers */
static struct cc_log_driver_st *curdrvr = NULL; /* Current driver  */
static struct cc_log_driver_st *nextdrv = NULL; /* Futur driver    */

static struct cc_log_code_st log_priorities[] = {
	LOC_CODE_ENTRY("ALERT",    LOG_ALERT   ),
	LOC_CODE_ENTRY("CRIT",     LOG_CRIT    ),
	LOC_CODE_ENTRY("DEBUG",    LOG_DEBUG   ),
	LOC_CODE_ENTRY("EMERG",    LOG_EMERG   ),
	LOC_CODE_ENTRY("ERR",      LOG_ERR     ),
	LOC_CODE_ENTRY("INFO",     LOG_INFO    ),
	LOC_CODE_ENTRY("NOTICE",   LOG_NOTICE  ),
	LOC_CODE_ENTRY("WARNING",  LOG_WARNING ),
};

/* Local global definitions */

#define FLOGGER(name, level) void cc_log_##name(const char *format, ...) { va_list ap; va_start(ap, format); do_log(level, format, ap);	va_end(ap); return; }
FLOGGER(alert,   LOG_ALERT   );
FLOGGER(crit,    LOG_CRIT    );
FLOGGER(debug,   LOG_DEBUG   );
FLOGGER(emerg,   LOG_EMERG   );
FLOGGER(err,     LOG_ERR     );
FLOGGER(info,    LOG_INFO    );
FLOGGER(notice,  LOG_NOTICE  );
FLOGGER(warning, LOG_WARNING );
#undef FLOGGER

#define VLOGGER(name, level) void cc_log_v##name(const char *format, va_list ap) { do_log(level, format, ap); return; }
VLOGGER(alert,   LOG_ALERT   );
VLOGGER(crit,    LOG_CRIT    );
VLOGGER(debug,   LOG_DEBUG   );
VLOGGER(emerg,   LOG_EMERG   );
VLOGGER(err,     LOG_ERR     );
VLOGGER(info,    LOG_INFO    );
VLOGGER(notice,  LOG_NOTICE  );
VLOGGER(warning, LOG_WARNING );
#undef VLOGGER

void cc_log_close(void)
{
	if(curdrvr)
		curdrvr->ld_close();
	return;
}

int cc_log_config(const char *attribute, const char *value)
{
	if(0 == strcasecmp("level", attribute))
	{
		const struct cc_log_code_st *ptr;
		if(NULL == (ptr = cc_log_find_name(value, log_priorities)))
		{
			cc_log_err("cc_log_config: Unknown level name '%s'", value);
			return -1;
		}
		loglevel = ptr->c_val;
		return 0;
	}

	if(0 == strcasecmp("timeformat", attribute))
	{
		const char *ptr = (const char *)cc_strdup(value);
		if(NULL == ptr)
		{
			cc_log_err("cc_log_config: Cannot allocate memory for time format '%s'", value);
			return -1;
		}
		if(tfmt_def != timefmt)
			cc_free((void *)timefmt);
		timefmt = ptr;
		return 0;
	}

	if(0 == strcasecmp("type", attribute))
	{
		struct cc_log_driver_st *ptr;
		for(ptr = drivers; ptr; ptr = ptr->ld_next)
		{
			if(0 == strcasecmp(ptr->ld_name, attribute))
				goto found;
		}
		cc_log_err("cc_log_config: Unknown log type '%s'", attribute);
		return -1;
	found:
		nextdrv = ptr;
		return 0;
	}

	return nextdrv->ld_config(attribute, value);
}

size_t cc_log_format_message (char *buffer, size_t bufsiz, int level, const char *format, va_list ap)
{
	static const struct cc_log_code_st lvl_default[] = { LOC_CODE_ENTRY("UNKNOWN", 0) };

	struct tm           tm[1];
	time_t              curtime;
	char               *bufptr = buffer;
	size_t              buffre = bufsiz;
	size_t              wrtsiz;

	time(&curtime);
	(void)localtime_r(&curtime, tm);
	wrtsiz = strftime(bufptr, buffre, timefmt, tm);
	bufptr += wrtsiz, buffre -= wrtsiz;
	if(buffre) *(bufptr++) = ' ', buffre -= 1;
	if(buffre) *(bufptr++) = '[', buffre -= 1;
	if(buffre)
	{
		const struct cc_log_code_st *plvl;
		if(NULL == (plvl = cc_log_find_code(level, log_priorities)))
			plvl = lvl_default;
		wrtsiz = CC_MIN(plvl->c_nlen, buffre);
		(void)memcpy(bufptr, plvl->c_name, wrtsiz);
		bufptr += wrtsiz, buffre -= wrtsiz;
	}
	if(buffre) *(bufptr++) = ']', buffre -= 1;
	if(buffre) *(bufptr++) = ' ', buffre -= 1;
	if(buffre)
	{
		wrtsiz = vsnprintf(bufptr, buffre, format, ap);
		bufptr += wrtsiz, buffre -= wrtsiz;
	}
	return bufsiz - buffre;
}

void cc_log_reinit(void)
{
	loglevel   = default_lvl;
	if(tfmt_def != timefmt)
		cc_free((void *)timefmt);
	timefmt    = tfmt_def;
	nextdrv->ld_reinit();
	cc_log_start();
	return;
}

void cc_log_start(void)
{
	curdrvr->ld_close();
	curdrvr = nextdrv;
	curdrvr->ld_open();
	return;
}

void cc_log_register_driver(struct cc_log_driver_st *driver)
{
	driver->ld_next = drivers, drivers = driver;
	return;
}


const struct cc_log_code_st *cc_log_search_code(int code, const struct cc_log_code_st *table, size_t tabsz)
{
	register const struct cc_log_code_st *curptr;
	register const struct cc_log_code_st *endptr;

	for(curptr = table, endptr = table + tabsz; curptr < endptr; curptr += 1)
		if(code == curptr->c_val)
			return curptr;
	return NULL;
}

const struct cc_log_code_st *cc_log_search_name(const char *name, const struct cc_log_code_st *table, size_t tabsz)
{
	register const struct cc_log_code_st *curptr;
	register const struct cc_log_code_st *endptr;

	for(curptr = table, endptr = table + tabsz; curptr < endptr; curptr += 1)
		if(0 == strcasecmp(curptr->c_name, name))
			return curptr;
	return NULL;
}

static void do_log(int level, const char *format, va_list ap)
{
	if(level <= loglevel)
		curdrvr->ld_write(level, format, ap);
	return;
}

/* STDERR driver */
static void stderr_open  (void) { return; }
static void stderr_close (void) __attribute__ ((weakref ("stderr_open")));
static void stderr_reinit(void) __attribute__ ((weakref ("stderr_open")));
static void stderr_write(int level, const char *format, va_list ap) {
	char   wbuffer[128];
	size_t towrite;
	towrite = cc_log_format_message(wbuffer, sizeof(wbuffer), level, format, ap);
	if(towrite < sizeof(wbuffer))
	{
		wbuffer[towrite++] = '\n';
		(void)CC_IO_WRITE(STDERR_FILENO, wbuffer, towrite);
	}
	else
	{
		(void)CC_IO_WRITE(STDERR_FILENO, wbuffer, towrite);
		(void)CC_IO_WRITE(STDERR_FILENO, "\n", 1);
	}
	return;
}

static int stderr_config(const char *attribute, const char *value)
{
	(void)attribute;
	(void)value;
	return -1;
}

static struct cc_log_driver_st serrdrv = {
	.ld_next    = NULL,
	.ld_name    = "stderr",
	.ld_open    = stderr_open,
	.ld_close   = stderr_close,
	.ld_write   = stderr_write,
	.ld_config  = stderr_config,
	.ld_reinit  = stderr_reinit
};

static __attribute__((constructor)) void drv_ctor(void)
{
	curdrvr = nextdrv = &serrdrv;
	cc_log_register_driver(&serrdrv);
	return;
}
