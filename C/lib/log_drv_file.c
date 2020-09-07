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
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <CCA/io.h>
#include <CCA/util.h>
#include <CCA/memory.h>

#include "log_internal.h"

static void file_open(void);
static void file_close(void);
static void file_write(int , const char *, va_list);
static int  file_config(const char *, const char *);

static const char *filename_default = "/tmp/cc_log.log";
static const char *filename = NULL;
static int         filedesc = -1;
static int         filemode = 0640;

static void file_open(void)
{
	file_close();

	if(NULL == filename)
	{
		cc_log_warning("file_open: No filename given ... assuming '%s'", filename_default);
		filename = filename_default;
	}
	if(-1 == (filedesc = open(filename, O_APPEND | O_WRONLY)) && -1 == (filedesc = open(filename, O_CREAT | O_WRONLY, filemode)))
	{
		cc_log_err("file_open: Cannot open '%s' for write ... errno = %d (%s)", filename, errno, strerror(errno));
		exit(1);
	}
	return;
}

static void file_close(void)
{
	if(-1 != filedesc)
		close(filedesc);
	filedesc = -1;
	return;
}

static void file_write(int level, const char *format, va_list ap)
{
	char   wbuffer[128];
	size_t towrite;
	towrite = cc_log_format_message(wbuffer, sizeof(wbuffer), level, format, ap);
	if(towrite < sizeof(wbuffer))
	{
		wbuffer[towrite++] = '\n';
		(void)CC_IO_WRITE(filedesc, wbuffer, towrite);
	}
	else
	{
		(void)CC_IO_WRITE(filedesc, wbuffer, towrite);
		(void)CC_IO_WRITE(filedesc, "\n", 1);
	}
	return;
}

static int file_config(const char *attribute, const char *value)
{
	if(0 == strcasecmp("file", attribute))
	{
		const char *ptr;
		if(NULL == (ptr = (const char *)cc_strdup(value)))
		{
			cc_log_err("file_config: Cannot allocate memory for file = '%'", value);
			return -1;
		}
		if(filename != filename_default)
			cc_free((void *)filename);
		filename = ptr;
		return 0;
	}

	if(0 == strcasecmp("mode", attribute))
	{
		char     *r;
		long int  v;
		v = strtol(value, &r, 0);
		if((r && *r) || v < 0 || v > 0777)
		{
			cc_log_err("file_config: bad file mode %s", value);
			return -1;
		}
		filemode = (int)v;
		return 0;
	}

	cc_log_err("file_config: Unknown attrinute '%s'", attribute);
	return -1;
}

static void file_reinit(void)
{
	if(filename != filename_default)
		cc_free((void *)filename);
	filename = NULL;
	filemode = 0640;
	
}

static struct cc_log_driver_st slogdrv = {
	.ld_next    = NULL,
	.ld_name    = "file",
	.ld_open    = file_open,
	.ld_close   = file_close,
	.ld_write   = file_write,
	.ld_config  = file_config,
	.ld_reinit  = file_reinit
};

static __attribute__((constructor)) void drv_ctor(void)
{
	cc_log_register_driver(&slogdrv);
	return;
}
