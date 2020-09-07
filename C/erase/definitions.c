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

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>

#include <CCA/fmt.h>
#include <CCA/memory.h>
#include <CCA/util.h>

#include "erase_internal.h"

/*
 * File inclusion:
 * ===============
 *
 * include file_path
 * Wildcard matching is allowed.
 *
 * Method definitions:
 * ===================
 *
 * method <name> <description>
 *  <passes definitions>
 * end
 *
 * Passes definitions:
 * ===================
 *
 * byte <hh>:
 * byte random each|once:
 * invert:
 * method <name>:
 * one:
 * pattern <pattern>:
 * random each|once:
 * rotate bit|byte left|right <n>:
 * zero:
 *
 */

#define DEF_BUFFER_REALSIZE BUFFER_ROUNDUP(DEF_BUFSIZE + 1)

extern erase_status_t erase_definition_read_from_fd(int, const char *, size_t);
extern erase_status_t erase_definition_read_from_file(const char *);
extern erase_status_t erase_definition_write_to_fd(int, const char *, size_t, erase_method_t *, int);
extern erase_status_t erase_definition_write_to_file(const char *, erase_method_t *, int);
extern erase_status_t erase_definition_write_all_fd(int, const char *, size_t);
extern erase_status_t erase_definition_write_all_file(const char *);

struct context_st;
struct keyword_st;

typedef erase_status_t (*kw_func_t)(struct context_st *, struct keyword_st *, char *, char *);

typedef struct context_st {
	int             ct_is_top;
	int             ct_filedesc;
	const char     *ct_filename;
	size_t          ct_lineno;
	erase_method_t *ct_method;
	char           *ct_bufpos;
	char           *ct_buffre;
	unsigned long   ct_buffer[DEF_BUFFER_REALSIZE];
} context_t;

typedef struct included_st {
	struct included_st *in_next;
	dev_t               in_dev;
	ino_t               in_ino;
} included_t;

typedef struct keyword_st {
	const char *kw_name;	/* Keyword name        */
	long        kw_info;	/* User informations   */
	kw_func_t   kw_func;	/* Associated function */
} keyword_t;

#define KW_FUNC_NULL (kw_func_t)NULL
#define KW_FUNC_END  (kw_func_t)-1

static erase_status_t  bad_argument(context_t *, const char *, const char *);
static erase_status_t  bind_pass(context_t *, const char *, erase_pass_t *pass);
static erase_status_t  byte_value(context_t *, unsigned long *, const char *, int);
static erase_status_t  context_create(int filedesc, const char *filename, size_t lineno, context_t **context);
static erase_status_t  context_destroy(context_t *);
static erase_status_t  malformed(context_t *, const char *);
static erase_status_t  new_pass_fail(context_t *, const char *, erase_status_t);
static erase_status_t  process_lines(context_t *, const keyword_t *);
static erase_status_t  readline(context_t *, char **);
static size_t          split(char **, char **, size_t, int);
static erase_status_t  subprocess_lines(context_t *, const keyword_t *);
static erase_status_t  warning(context_t *, erase_status_t, const char *, ...);
static erase_status_t  write_method(context_t *, erase_method_t *, int);

static erase_status_t  kw_main_include(context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_main_method (context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_01     (context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_byte   (context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_invert (context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_method (context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_pattern(context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_random (context_t *, keyword_t *, char *, char *);
static erase_status_t  kw_meth_rotate (context_t *, keyword_t *, char *, char *);


keyword_t kw_main[] = {
	{ "include",    0, kw_main_include },
	{ "method",     0, kw_main_method  },
	{ NULL,         0, NULL }
};

keyword_t kw_meth[] = {
	{ "byte",       0, kw_meth_byte    },
	{ "end",        0, KW_FUNC_END     },
	{ "invert",     0, kw_meth_invert  },
	{ "method",     0, kw_meth_method  },
	{ "one",     0xFF, kw_meth_01      },
	{ "pattern",    0, kw_meth_pattern },
	{ "random",     0, kw_meth_random  },
	{ "rotate",     0, kw_meth_rotate  },
	{ "zero",    0x00, kw_meth_01      },
	{ NULL,         0, NULL }
};

static const char  ifs[] = " \t\n\r";
static included_t *included_files = CC_TNULL(included_t);

static int fmt_string(char **buffer, size_t *bufsiz, const char *string)
{
	return (ssize_t)strlen(string) != cc_fmt_string(buffer, bufsiz, string) ? -1 : 0;
}

static int fmt_hexdump(char **buffer, size_t *bufsiz, const void *data, size_t dsize)
{
	ssize_t esize = (ssize_t)(dsize * 2);
	return esize != cc_fmt_hexdump(buffer, bufsiz, (unsigned char *)data, dsize) ? -1 : 0;
}

static erase_status_t  bad_argument(context_t *context, const char *directive, const char *argument)
{
	return warning(context, ERA_ST_SYNTAX_ERROR, "%s: bad argument `%s'", directive, argument);
}

static erase_status_t  bind_pass(context_t *context, const char *directive, erase_pass_t *pass)
{
	erase_status_t status;
	status = erase_method_add_pass(context->ct_method, pass);
	return ERA_ST_OK == status ? status : warning(context, status, "%s: cannot bind pass to the method `%s'", directive, context->ct_method->em_name);
}

static erase_status_t  byte_value(context_t *context, unsigned long *value, const char *string, int strict)
{
	char        b[3];
	char       *d;
	const char *s;
	long        v;
	s = string;
	d = b;
	if('\0' == *s) goto badarg; *(d++) = *(s++);
	if('\0' == *s) goto badarg; *(d++) = *(s++);
	*d = '\0';
	if(strict && '\0' != *s)
		goto badarg;
	v = strtol(b, &d, 16);
	if(v < 0x00 || v > 0xFF || (d && '\0' != *d))
		return warning(context, ERA_ST_VALUE_ERROR, "%s: Not a valid hexadecimal byte value", b);
	*value = (unsigned long)v;
	return ERA_ST_OK;
badarg:
	return warning(context, ERA_ST_VALUE_ERROR, "%s: Not a valid hexadecimal byte value", string);
}

static erase_status_t context_create(int filedesc, const char *filename, size_t lineno, context_t **context)
{
	char           *fname;
	erase_status_t  status;

	fname = CC_TNULL(char);
	if(filename && ERA_ST_OK != (status = _erase_strdup(filename, &fname)))
		return status;

	if(ERA_ST_OK == (status = _erase_memory_alloc(sizeof(context_t), (void **)context)))
	{
		(*context)->ct_is_top   = 1;
		(*context)->ct_filedesc = filedesc;
		(*context)->ct_filename = fname;
		(*context)->ct_lineno   = lineno;
		(*context)->ct_method   = CC_TNULL(erase_method_t);
		(*context)->ct_bufpos   = (char *)(*context)->ct_buffer;
		(*context)->ct_buffre   = (char *)(*context)->ct_buffer;
	}
	else
		CC_PROTECT_ERRNO((void)_erase_memory_free(fname));
	return status;
}

static erase_status_t context_destroy(context_t *context)
{
	if(context->ct_filename)
		(void)_erase_memory_free((void *)(context->ct_filename));
	return _erase_memory_free((void *)context);
}

static erase_status_t malformed(context_t *context, const char *directive)
{
	return warning(context, ERA_ST_SYNTAX_ERROR, "%s: malformed directive", directive);
}

static erase_status_t new_pass_fail(context_t *context, const char *directive, erase_status_t status)
{
	return warning(context, status, "%s: cannot create new pass", directive);
}

static erase_status_t process_lines(context_t *context, const keyword_t *keywords)
{
	char           *line;
	char           *rest;
	char           *word;
	erase_status_t  status;
	keyword_t      *keyword;

	while(ERA_ST_OK == (status = readline(context, &line)))
	{
		word = strtok_r(line, ifs, &rest);
		if(!word || *word == '\0' || *word == '#')
			continue;
		for(keyword = (keyword_t *)keywords; keyword->kw_name; keyword += 1)
			if(0 == strcmp(word, keyword->kw_name))
				goto found;
		return warning(context, ERA_ST_SYNTAX_ERROR, "unknown keyword %s", word);
	found:
		if(KW_FUNC_END == keyword->kw_func)
		{
			if(context->ct_is_top)
				return warning(context, ERA_ST_SYNTAX_ERROR, "misplaced %s directive", keyword->kw_name);
			return ERA_ST_OK;
		}
		if(ERA_ST_OK != (status = keyword->kw_func(context, keyword, word, rest)))
			return status;
	}
	return (context->ct_is_top) ? ERA_ST_OK : warning(context, ERA_ST_PREMATURE_END_OF_FILE, "premature end of file");
}

static erase_status_t _readline(context_t *context, char **retv)
{
	char *rval;
	char *cpos;
	char *epos;

	epos = context->ct_buffre;
	for(rval = context->ct_bufpos; rval < epos && isspace(*rval) && *rval != '\n'; rval += 1);
	for(cpos = rval; cpos < epos; cpos += 1)
	{
		if(*cpos == '\n')
		{
			*(cpos) = '\0';
			context->ct_bufpos  = cpos + 1;
			context->ct_lineno += 1;
			while(cpos > rval && isspace(*cpos)) *(cpos--) = '\0';
			*retv = rval;
			return ERA_ST_OK;
		}
	}
	return ERA_ST_LINE_TOO_LONG;
}

static erase_status_t _refill_buffer(context_t *context)
{
	size_t         nbytes;
	ssize_t        nread;
	erase_status_t status;

	status = ERA_ST_OK;

	if(context->ct_bufpos < context->ct_buffre)
	{
		size_t used = context->ct_buffre - context->ct_bufpos;
		(void)memcpy(context->ct_buffer, context->ct_bufpos, used);
		nbytes = DEF_BUFSIZE - used;
		context->ct_bufpos = (char *)context->ct_buffer;
		context->ct_buffre = context->ct_bufpos + used;
	}
	else
	{
		context->ct_bufpos = (char *)context->ct_buffer;
		context->ct_buffre = (char *)context->ct_buffer;
		nbytes = DEF_BUFSIZE;
	}
	nread = read(context->ct_filedesc, context->ct_buffre, nbytes);
	switch(nread)
	{
	case  0:
		status = ERA_ST_END_OF_FILE;
		break;
	case -1:
		status = ERA_ST_SYSTEM_ERROR;
		break;
	default:
		context->ct_buffre += nread;
		*(context->ct_buffre) = '\0';
	}
	return status;
}

static erase_status_t readline(context_t *context, char **retv)
{
	erase_status_t status;
	if(ERA_ST_OK == _readline(context, retv))
		return ERA_ST_OK;
	status = _refill_buffer(context);
	if(ERA_ST_OK != status)
	{
		if(ERA_ST_END_OF_FILE == status && context->ct_bufpos < context->ct_buffre)
		{
			*retv = context->ct_bufpos;
			return ERA_ST_OK;
		}
		return status;
	}
	return _readline(context, retv);
}

static size_t split(char **line, char **args, size_t nargs, int strict)
{
	char    *li;
	char    *re;
	char   **ca;
	char    *na;
	size_t   nb;

	for(li = na = *line, ca = args, nb = 0; na && nb < nargs; li = CC_TNULL(char), nb += 1, ca += 1)
	{
		na = strtok_r(li, ifs, &re);
		if(CC_TNULL(char) == na)
			break;
		*ca = na;
	}
	*line = re;
	if(strict && (nb < nargs || (re && *re != '\0')))
	{
		return 0;
	}
	return nb;
}

static erase_status_t subprocess_lines(context_t *context, const keyword_t *keywords)
{
	int            is_top;
	erase_status_t status;
	is_top = context->ct_is_top;
	context->ct_is_top = 0;
	status = process_lines(context, keywords);
	context->ct_is_top = is_top;
	return status;
}

static erase_status_t warning(context_t *context, erase_status_t retv, const char *format, ...)
{
	char    bu[128];
	va_list ap;
	int     er;

	er = errno;
	va_start(ap, format);
	vsnprintf(bu, sizeof(bu), format, ap);
	va_end(ap);
	fprintf(stderr, "Warning %s [%lu]: %s\n", CC_IFNULL(char, context->ct_filename, ""), (unsigned long)(context->ct_lineno), bu);
	errno = er;
	return retv;
}

static erase_status_t _flush_buffer(context_t *context, size_t size)
{
	size_t   tw;
	ssize_t  nw;
	char    *wp;
	for(tw = size, nw = 0, wp = (char *)(context->ct_buffer); tw > 0; tw -= nw, wp += nw)
		if(1 > (nw = write(context->ct_filedesc, wp, tw)))
			return ERA_ST_SYSTEM_ERROR;
	return ERA_ST_OK;
}

static erase_status_t write_method(context_t *context, erase_method_t *method, int complete)
{
	unsigned long    arg;
	unsigned long    cod;
	size_t           buflen;
	erase_pass_t   **cpass;
	size_t           npass;
	erase_status_t   status;
	char            *buffer;
	char            *bufptr;

	if(method->em_marker)
		return ERA_ST_OK;

	if(complete)
	{
		for(cpass = method->em_passes, npass = method->em_npass; npass > 0; npass -= 1, cpass += 1)
			if((ERASE_MODE_METHOD_CODE == ERASE_MODE_CODE((*cpass)->ep_mode)) &&
			   (ERA_ST_OK != (status = write_method(context, method, complete))))
				return status;
	}
	
	method->em_marker = 1;

	buffer = (char *)(context->ct_buffer);
	bufptr = (char *)(context->ct_buffer);
	buflen = DEF_BUFSIZE;
	if(fmt_string(&bufptr, &buflen, "method ")       ||
	   fmt_string(&bufptr, &buflen, method->em_name) ||
	   fmt_string(&bufptr, &buflen, " ")             ||
	   fmt_string(&bufptr, &buflen, method->em_desc) ||
	   0 == cc_fmt_crlf(&bufptr, &buflen))
		return ERA_ST_LINE_TOO_LONG;

	if(ERA_ST_OK != (status = _flush_buffer(context, bufptr - buffer)))
		return status;

	for(cpass = method->em_passes, npass = method->em_npass; npass > 0; cpass += 1)
	{
		bufptr = (char *)(context->ct_buffer);
		buflen = DEF_BUFSIZE;
		if(0 == cc_fmt_char(&bufptr, &buflen, '\t'))
			return ERA_ST_LINE_TOO_LONG;
		switch((cod = ERASE_MODE_CODE((*cpass)->ep_mode)))
		{
		case ERASE_MODE_BYTE_CODE:
			switch((arg = ERASE_MODE_ARG((*cpass)->ep_mode)))
			{
			case 0x00UL:
				if(fmt_string(&bufptr, &buflen, "zero"))
					return ERA_ST_LINE_TOO_LONG;
				break;
			case 0xFFUL:
				if(fmt_string(&bufptr, &buflen, "one"))
					return ERA_ST_LINE_TOO_LONG;
				break;
			default:
				if(fmt_string(&bufptr, &buflen, "byte ") || 0 == cc_fmt_ulong(&bufptr, &buflen, arg, 16))
					return ERA_ST_LINE_TOO_LONG;
				break;
			}
			break;
		case ERASE_MODE_RNDBYTE_CODE:
			if(fmt_string(&bufptr, &buflen, "byte random "))
				return ERA_ST_LINE_TOO_LONG;
			if(fmt_string(&bufptr, &buflen, 0 == ERASE_MODE_ARG((*cpass)->ep_mode) ? "once" : "each"))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_RANDOM_CODE:
			if(fmt_string(&bufptr, &buflen, "random "))
				return ERA_ST_LINE_TOO_LONG;
			if(fmt_string(&bufptr, &buflen, 0 == ERASE_MODE_ARG((*cpass)->ep_mode) ? "once" : "each"))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_PATTERN_CODE:
			if(fmt_string(&bufptr, &buflen, "pattern "))
				return ERA_ST_LINE_TOO_LONG;
			if(fmt_hexdump(&bufptr, &buflen, (*cpass)->ep_pattern_data, (*cpass)->ep_pattern_size))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_INVERT_CODE:
			if(fmt_string(&bufptr, &buflen, "invert"))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_BIT_LROTATE_CODE:
			if(fmt_string(&bufptr, &buflen, "rotate bit left "))
				return ERA_ST_LINE_TOO_LONG;
			if(0 == cc_fmt_ulong(&bufptr, &buflen, ERASE_MODE_ARG((*cpass)->ep_mode), 10))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_BIT_RROTATE_CODE:
			if(fmt_string(&bufptr, &buflen, "rotate bit right "))
				return ERA_ST_LINE_TOO_LONG;
			if(0 == cc_fmt_ulong(&bufptr, &buflen, ERASE_MODE_ARG((*cpass)->ep_mode), 10))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_BYTE_LROTATE_CODE:
			if(fmt_string(&bufptr, &buflen, "rotate byte left "))
				return ERA_ST_LINE_TOO_LONG;
			if(cc_fmt_ulong(&bufptr, &buflen, ERASE_MODE_ARG((*cpass)->ep_mode), 10))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_BYTE_RROTATE_CODE:
			if(fmt_string(&bufptr, &buflen, "rotate byte right "))
				return ERA_ST_LINE_TOO_LONG;	
			if(0 == cc_fmt_ulong(&bufptr, &buflen, ERASE_MODE_ARG((*cpass)->ep_mode), 10))
				return ERA_ST_LINE_TOO_LONG;
			break;
		case ERASE_MODE_METHOD_CODE:
			if(fmt_string(&bufptr, &buflen, "method ") || fmt_string(&bufptr, &buflen, (*cpass)->ep_method_def->em_name))
				return ERA_ST_LINE_TOO_LONG;
			break;
		default:
			fprintf(stderr, "Unknown pass code %02lX\n", cod);
			return ERA_ST_LINE_TOO_LONG;
		}
		if(0 == cc_fmt_crlf(&bufptr, &buflen))
			return ERA_ST_LINE_TOO_LONG;
		if(ERA_ST_OK != (status = _flush_buffer(context, bufptr - buffer)))
			return status;
	}
	return ERA_ST_OK;
}

/*
 * Kewwords functions:
 * ===================
 */
static int _glob_error(const char *epath, int eerrno)
{
	CC_PROTECT_ERRNO(fprintf(stderr, "%s: %s\n", epath, strerror(eerrno)));
	return 1;
}

static erase_status_t kw_main_include(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	int              gret;
	glob_t           gval;
	char            *argv[1];
	size_t           argc;
	erase_status_t   status;
	size_t           pathc;
	char           **pathv;

	(void)keyword;
	erase_debug("kw_main_include(%p, %p, %s, %s", context, keyword, arg0, args);
	if(0 == (argc = split(&args, argv, 1, 1)))
		return malformed(context, arg0);
	gval.gl_pathc = 0;
	gval.gl_pathv = CC_TNULL(char *);
	gval.gl_offs  = 0;
	if(0 != (gret = glob(*argv, GLOB_ERR | GLOB_BRACE | GLOB_TILDE, _glob_error, &gval)))
		switch(gret)
		{
		case GLOB_NOSPACE:
			return warning(context, ERA_ST_SYSTEM_ERROR, "%s: insufisant memory space", arg0);
		case GLOB_ABORTED:
			return warning(context, ERA_ST_SYSTEM_ERROR, "%s: cannot expand %s", arg0, *argv);
		case GLOB_NOMATCH:
			return warning(context, ERA_ST_VALUE_ERROR, "%s: no match for %s", arg0, *argv);
		default:
			return warning(context, ERA_ST_INTERNAL, "%s: unknown glob(3) error %d", arg0, gret);
		}
	for(pathv = gval.gl_pathv, pathc = 0; pathc < gval.gl_pathc; pathc += 1, pathv += 1)
		if(ERA_ST_OK != (status = erase_definition_read_from_file(*pathv)))
		{
			(void)warning(context, status, "%s: cannot process file %s", arg0, *pathv);
			break;
		}
	CC_PROTECT_ERRNO(globfree(&gval));
	return status;
}

static erase_status_t kw_main_method(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	char           *name;
	char           *description;
	erase_method_t *method;
	erase_status_t  status;

	(void)keyword;
	erase_debug("kw_main_method(%p, %p, %s, %s", context, keyword, arg0, args);
	name = strtok_r(args, ifs, &description);
	if(CC_TNULL(char) == name || '\0' == *name || CC_TNULL(char) == description || '\0' == *description)
		return warning(context, ERA_ST_VALUE_ERROR, "%s: incomplete directive", arg0);
	if(ERA_ST_OK != (status = erase_method_create(name, description, &method)))
		return warning(context, status, "%s: cannot allocate new method", arg0);
	if(ERA_ST_OK != (status = erase_method_register(method)))
		return warning(context, status, "%s: `%s' %s", arg0, name, erase_error_string(status));
	context->ct_method = method;
	status = subprocess_lines(context, kw_meth);
	context->ct_method = CC_TNULL(erase_method_t);
	return status;
}

static erase_status_t kw_meth_01(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	erase_status_t  status;
	erase_pass_t   *pass;
	(void)keyword;
	erase_debug("kw_meth_01(%p, %p, '%s', '%s')", context, keyword, arg0, args);
	if(args && '\0' != *args)
		return malformed(context, arg0);
	status = erase_pass_create_byte((unsigned long)(keyword->kw_info), &pass);
	return ERA_ST_OK != status ? new_pass_fail(context, arg0, status) : bind_pass(context, arg0, pass);
}

static erase_status_t kw_meth_byte(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	size_t          argc;
	char           *argv[2];
	erase_pass_t   *pass;
	erase_status_t  status;

	(void)keyword;
	erase_debug("kw_meth_byte(%p, %p, '%s', '%s')", context, keyword, arg0, args);
	argc = split(&args, argv, 2, 0);
	if((argc != 1 && argc != 2) || (args && *args != '\0'))
		return malformed(context, arg0);
	if(argc == 1)
	{
		unsigned long byte;
		if(ERA_ST_OK != (status = byte_value(context, &byte, argv[0], 1)))
			return warning(context, status, "%s: value %s not a valid hexadecimal byte value", arg0, argv[0]);
		status = erase_pass_create_byte(byte, &pass);
	}
	else
	{
		int each = -1;
		erase_debug("kw_meth_byte: argv[0] = %s, argv[1] = %s\n", argv[0], argv[1]);
		if(strcmp("random", argv[0]))
			return bad_argument(context, arg0, argv[0]);
		if(0 == strcmp("each", argv[1])) each = 1;
		if(0 == strcmp("once", argv[1])) each = 0;
		if(-1 == each)
			return bad_argument(context, arg0, argv[1]);
		status = erase_pass_create_rndbyte(each, &pass);
	}
	return ERA_ST_OK != status ? new_pass_fail(context, arg0, status) : bind_pass(context, arg0, pass);
}

static erase_status_t kw_meth_invert(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	erase_status_t  status;
	erase_pass_t   *pass;
	(void)keyword;
	erase_debug("kw_meth_invert(%p, %p, %s, %s", context, keyword, arg0, args);
	if(args && '\0' != *args)
		return malformed(context, arg0);
	status = erase_pass_create_invert(&pass);
	return ERA_ST_OK != status ? new_pass_fail(context, arg0, status) : bind_pass(context, arg0, pass);
}

static erase_status_t kw_meth_method(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	size_t          argc;
	char           *argv[2];
	erase_method_t *method;
	erase_pass_t   *pass;
	erase_status_t  status;

	(void)keyword;
	erase_debug("kw_meth_method(%p, %p, %s, %s", context, keyword, arg0, args);
	if(0 == (argc = split(&args, argv, 1, 1)))
		return malformed(context, arg0);
	if(ERA_ST_OK == (status = erase_method_find(argv[0], &method)))
	{
		status = erase_pass_create_method(method, &pass);
	}
	else
	{
		if(OPTION_BACK_NAME_RESOLUTION != (_erase_options & OPTION_BACK_NAME_RESOLUTION))
			return warning(context, status, "%s: unknow method `%s'", arg0, argv[0]);
		status = erase_pass_create_method_name(argv[0], &pass);
	}
	return ERA_ST_OK != status ? new_pass_fail(context, arg0, status) : bind_pass(context, arg0, pass);
}

static erase_status_t kw_meth_pattern(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	size_t          argc;
	char           *argv[1];
	erase_pass_t   *pass;
	erase_status_t  status;
	size_t          pat_size;
	unsigned char  *d;
	char           *s;
	size_t          i;
	unsigned long   b;

	(void)keyword;
	erase_debug("kw_meth_pattern(%p, %p, %s, %s", context, keyword, arg0, args);
	if(0 == (argc = split(&args, argv, 1, 1)))
		return malformed(context, arg0);

	pat_size = strlen(argv[0]);
	if(1 == (pat_size & 1))
		return bad_argument(context, arg0, argv[0]);
	pat_size /= 2;

	if(ERA_ST_OK != (status = erase_pass_create_pattern((unsigned char *)argv[0], pat_size, &pass)))
		return new_pass_fail(context, arg0, status);

	for(s = argv[0], d = pass->ep_pattern_data, i = 0; i < pat_size; s += 2, d += 1, i -= 1)
	{
		if(ERA_ST_OK != (status = byte_value(context, &b, s, 0)))
			return bad_argument(context, arg0, argv[0]);
		*(d) = (unsigned char)(b & 0xFF);
	}
	return bind_pass(context, arg0, pass);
}

static erase_status_t kw_meth_random(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	erase_status_t   status;
	char            *argv[1];
	size_t           argc;
	erase_pass_t    *pass;
	int              each;

	erase_debug("kw_meth_random(%p, %p, %s, %s", context, keyword, arg0, args);
	if(0 == (argc = split(&args, argv, 1, 1)))
		return malformed(context, arg0);
	each = -1;
	if( 0 == strcmp("each", argv[0])) each = 1;
	if( 0 == strcmp("once", argv[0])) each = 0;
	if(-1 == each)
		return bad_argument(context, arg0, argv[0]);
	status = erase_pass_create_random(each, &pass);
	return ERA_ST_OK != status ? new_pass_fail(context, arg0, status) : bind_pass(context, arg0, pass);
}

static erase_status_t kw_meth_rotate(context_t *context, keyword_t *keyword, char *arg0, char *args)
{
	erase_status_t (*pass_create[])(unsigned long, erase_pass_t **) = {
		erase_pass_create_bit_lrotate,
		erase_pass_create_bit_rrotate,
		erase_pass_create_byte_lrotate,
		erase_pass_create_byte_rrotate
	};
	erase_status_t   status;
	char            *argv[3];
	size_t           argc;
	erase_pass_t    *pass;
	long             nrot;
	char            *rest;
	int              i0;
	int              i1;

	(void)keyword;
	erase_debug("kw_meth_rotate(%p, %p, %s, %s", context, keyword, arg0, args);
	if(0 == (argc = split(&args, argv, 3, 1)))
		return malformed(context, arg0);
	i0 = i1 = -1;
	if(0 == strcmp("bit",   argv[0])) i0 = 0;
	if(0 == strcmp("byte",  argv[0])) i0 = 2;
	if(0 == strcmp("left",  argv[1])) i1 = 0;
	if(0 == strcmp("right", argv[1])) i1 = 1;
	if(-1 == i0) return bad_argument(context, arg0, argv[0]);
	if(-1 == i1) return bad_argument(context, arg0, argv[1]);
	nrot = strtol(argv[2], &rest, 0);
	if((rest && '\0' != *rest) || nrot < 0 || nrot > 255)
		return warning(context, ERA_ST_VALUE_ERROR, "%s: bad rotation value %s\n", argv[2]);
	status = pass_create[i0 | i1](nrot, &pass);
	return ERA_ST_OK != status ? new_pass_fail(context, arg0, status) : bind_pass(context, arg0, pass);
}

erase_status_t erase_definition_read_from_fd(int fd, const char *filename, size_t lineno)
{
	context_t      *context;
	erase_status_t  status;

	erase_debug("erase_definition_read_from_fd(%d, %s, %lu", fd, filename, lineno);
	if(ERA_ST_OK == (status = context_create(fd, filename, lineno, &context)))
	{
		status = process_lines(context, kw_main);
		CC_PROTECT_ERRNO((void)context_destroy(context));
	}
	return status;
}

erase_status_t erase_definition_read_from_file(const char *filename)
{
	int             fdesc;
	erase_status_t  status;
	included_t     *pincl;
	struct stat     finfo;

	erase_debug("erase_definition_read_from_file(%s)", filename);

	status = ERA_ST_SYSTEM_ERROR;

	if(-1 == stat(filename, &finfo))
	{
		CC_PROTECT_ERRNO(perror(filename));
		return status;
	}

	for(pincl = included_files; pincl; pincl = pincl->in_next)
		if(pincl->in_ino == finfo.st_ino && pincl->in_dev == finfo.st_dev)
			return ERA_ST_OK;

	if(ERA_ST_OK != (status = _erase_memory_alloc(sizeof(included_t), (void **)&pincl)))
		return status;
	pincl->in_next = included_files;
	pincl->in_dev  = finfo.st_dev;
	pincl->in_ino  = finfo.st_ino;
	included_files = pincl;

	if(-1 == (fdesc = open(filename, O_RDONLY)))
	{
		CC_PROTECT_ERRNO(perror(filename));
		return ERA_ST_SYSTEM_ERROR;
	}
	status = erase_definition_read_from_fd(fdesc, filename, 0);
	CC_PROTECT_ERRNO(close(fdesc));
	return status;
}

erase_status_t erase_definition_write_to_fd(int fd, const char *filename, size_t lineno, erase_method_t *method, int complete)
{
	context_t      *context;
	erase_status_t  status;

	erase_debug("erase_definition_write_to_fd(%d, %s, %lu, %s, %d", fd, filename, lineno, method, complete);
	if(ERA_ST_OK != (status = erase_method_validate(method)))			return status;
	if(ERA_ST_OK != (status = erase_methods_clear_marker()))			return status;
	if(ERA_ST_OK != (status = context_create(fd, filename, lineno, &context)))	return status;
	status = write_method(context, method, complete);
	CC_PROTECT_ERRNO((void)context_destroy(context));
	return status;
}

erase_status_t erase_definition_write_to_file(const char *filename, erase_method_t *method, int complete)
{
	int             fdesc;
	erase_status_t  status;

	erase_debug("erase_definition_write_to_file(%s, %p, %d)", filename, method, complete);
	if(-1 == (fdesc = open(filename, O_WRONLY)))
		return ERA_ST_SYSTEM_ERROR;
	status = erase_definition_write_to_fd(fdesc, filename, 0, method, complete);
	CC_PROTECT_ERRNO(close(fdesc));
	return status;
}

static erase_status_t _write_all_methods_cb(void *context, erase_method_t *method)
{
	return write_method((context_t *)context, method, 1);
}

erase_status_t erase_definition_write_all_fd(int fd, const char *filename, size_t lineno)
{
	erase_status_t  status;
	context_t      *context;

	erase_debug("erase_definition_write_all_fd(%d, %s, %lu)", fd, filename, lineno);
	if(ERA_ST_OK != (status = erase_method_validate_db()))				return status;
	if(ERA_ST_OK != (status = erase_methods_clear_marker()))			return status;
	if(ERA_ST_OK != (status = context_create(fd, filename, lineno, &context)))	return status;
	status = erase_method_walk((void *)context, _write_all_methods_cb);
	CC_PROTECT_ERRNO((void)context_destroy(context));
	return status;
}

erase_status_t erase_definition_write_all_file(const char *filename)
{
	int             fdesc;
	erase_status_t  status;
	erase_debug("erase_definition_write_all_file(%s)", filename);
	if(-1 == (fdesc = open(filename, O_WRONLY)))
		return ERA_ST_SYSTEM_ERROR;
	status = erase_definition_write_all_fd(fdesc, filename, 0);
	CC_PROTECT_ERRNO(close(fdesc));
	return status;
}
