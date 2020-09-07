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

#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>
#include <syslog.h>

#include <CCA/util.h>
#include <CCA/memory.h>

#include "log_internal.h"

static void syslog_open(void);
static void syslog_close(void);
static void syslog_write(int , const char *, va_list);
static int  syslog_config(const char *, const char *);

static struct cc_log_code_st syslog_facilities[] = {
	LOC_CODE_ENTRY("AUTH",     LOG_AUTH    ),
	LOC_CODE_ENTRY("AUTHPRIV", LOG_AUTHPRIV),
	LOC_CODE_ENTRY("CRON",     LOG_CRON    ),
	LOC_CODE_ENTRY("DAEMON",   LOG_DAEMON  ),
	LOC_CODE_ENTRY("FTP",      LOG_FTP     ),
	LOC_CODE_ENTRY("KERN",     LOG_KERN    ),
	LOC_CODE_ENTRY("LPR",      LOG_LPR     ),
	LOC_CODE_ENTRY("MAIL",     LOG_MAIL    ),
	LOC_CODE_ENTRY("NEWS",     LOG_NEWS    ),
	LOC_CODE_ENTRY("SYSLOG",   LOG_SYSLOG  ),
	LOC_CODE_ENTRY("USER",     LOG_USER    ),
	LOC_CODE_ENTRY("UUCP",     LOG_UUCP    ),
	LOC_CODE_ENTRY("LOCAL0",   LOG_LOCAL0  ),
	LOC_CODE_ENTRY("LOCAL1",   LOG_LOCAL1  ),
	LOC_CODE_ENTRY("LOCAL2",   LOG_LOCAL2  ),
	LOC_CODE_ENTRY("LOCAL3",   LOG_LOCAL3  ),
	LOC_CODE_ENTRY("LOCAL4",   LOG_LOCAL4  ),
	LOC_CODE_ENTRY("LOCAL5",   LOG_LOCAL5  ),
	LOC_CODE_ENTRY("LOCAL6",   LOG_LOCAL6  ),
	LOC_CODE_ENTRY("LOCAL7",   LOG_LOCAL7  ),
};

static struct cc_log_code_st syslog_options[] = {
        LOC_CODE_ENTRY("CONS",     LOG_CONS    ),
        LOC_CODE_ENTRY("NDELAY",   LOG_NDELAY  ),
        LOC_CODE_ENTRY("NOWAIT",   LOG_NOWAIT  ),
        LOC_CODE_ENTRY("ODELAY",   LOG_ODELAY  ),
        LOC_CODE_ENTRY("PERROR",   LOG_PERROR  ),
        LOC_CODE_ENTRY("PID",      LOG_PID     ),
};

static const char    default_identity[] = default_ide;
static const char   *slog_identity    = default_identity;
static int           slog_facility    = default_fac;
static int           slog_options     = default_opt;
static int           slog_opreset     = 1;

static void syslog_open(void)
{
	openlog(slog_identity, slog_options, slog_facility);
}

static void syslog_close(void)
{
	closelog();
	return;
}

static void syslog_write(int level, const char *format, va_list ap)
{
	vsyslog(LOG_MAKEPRI(slog_facility, level), format, ap);
	return;
}

static int syslog_config(const char *attribute, const char *value)
{
	if(0 == strcasecmp("facility", attribute))
	{
		const struct cc_log_code_st *ptr;
		if(NULL == (ptr = cc_log_find_name(value, syslog_facilities)))
		{
			cc_log_err("syslog_config: Unknown facility name '%s'", value);
			return -1;
		}
		slog_facility = ptr->c_val;
		return 0;
	}

	if(0 == strcasecmp("identity", attribute))
	{
		const char *ptr;
		if(NULL == (ptr = (const char *)cc_strdup(value)))
		{
			cc_log_err("syslog_config: Cannot allocate memory for identity = '%'", value);
			return -1;
		}
		if(slog_identity != default_identity)
			cc_free((void *)slog_identity);
		slog_identity = ptr;
	}

	if(0 == strcasecmp("option", attribute))
	{
		const struct cc_log_code_st *ptr;
		if(NULL == (ptr = cc_log_find_name(value, syslog_options)))
		{
			cc_log_err("syslog_config: Unknown option name '%s'", value);
			return -1;
		}
		if(slog_opreset)
			slog_opreset = slog_options = 0;
		slog_options |= ptr->c_val;
		return 0;
	}

	cc_log_err("syslog_config: Unknown attrinute '%s'", attribute);
	return -1;
}

static void syslog_reinit(void)
{
	if(default_identity != slog_identity)
		cc_free((void *)slog_identity);
	slog_identity = default_identity;
	slog_facility = default_fac;
	slog_options  = default_opt;
	slog_opreset  = 1;
}

static struct cc_log_driver_st slogdrv = {
	.ld_next    = NULL,
	.ld_name    = "syslog",
	.ld_open    = syslog_open,
	.ld_close   = syslog_close,
	.ld_write   = syslog_write,
	.ld_config  = syslog_config,
	.ld_reinit  = syslog_reinit
};

static __attribute__((constructor)) void drv_ctor(void)
{
	cc_log_register_driver(&slogdrv);
	return;
}
