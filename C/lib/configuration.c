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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>

#include <CCA/display.h>
#include <CCA/memory.h>
#include <CCA/util.h>

/* Local types */
struct cc_conf_kwr_st;

typedef struct cc_conf_ctx_st {
	int                          ct_is_top;
	int                          ct_filedesc;
	const char                  *ct_filename;
	size_t                       ct_lineno;
	const struct cc_conf_kwr_st *ct_keywords;
	char                        *ct_bufpos;
	char                        *ct_buffre;
	unsigned long                ct_buffer[1];
} cc_conf_ctx_t, *CC_CONF_CONTEXT;

typedef struct included_st {
	struct included_st *in_next;
	dev_t               in_dev;
	ino_t               in_ino;
} included_t;

#define __CC_CONFIGURATION_INTERNAL__
#include <CCA/configuration.h>

/* Global functions */

extern cc_conf_status_t  cc_conf_read      (const char *, const cc_conf_kwr_t *, void *);
extern size_t            cc_conf_set_bufsiz(size_t);
extern cc_conf_status_t  cc_conf_verror    (CC_CONF_CONTEXT, cc_conf_status_t, const char *, va_list);
extern cc_conf_status_t  cc_conf_error     (CC_CONF_CONTEXT, cc_conf_status_t, const char *, ...);
extern cc_conf_status_t  cc_conf_malformed (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t  cc_conf_perror    (CC_CONF_CONTEXT, const char *);
extern const char       *cc_conf_strerror  (cc_conf_status_t);
extern cc_conf_status_t  cc_conf_syntaxerr (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t  cc_conf_valueerr  (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t  cc_conf_systemerr (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t  cc_conf_enter_blk (CC_CONF_CONTEXT, const cc_conf_kwr_t *, void *);
extern cc_conf_status_t  cc_conf_include   (CC_CONF_CONTEXT, const cc_conf_kwr_t *, void *, int, char **);

/* Local functions */
static cc_conf_status_t  context_create (int, const cc_conf_kwr_t *, const char *, size_t, cc_conf_ctx_t **);
static void              context_destroy(cc_conf_ctx_t *);
static int               globing_error(const char *, int);
static cc_conf_status_t  process_lines  (cc_conf_ctx_t *, void *);
static cc_conf_status_t  read_line      (cc_conf_ctx_t *, char **);
static cc_conf_status_t  read_line0     (cc_conf_ctx_t *, char **);
static cc_conf_status_t  refill_buffer  (cc_conf_ctx_t *);
static cc_conf_status_t  split_line     (char *, char ***, int *, char **);

static size_t       bufsize = 1024;			  /* Buffer size    */
static const char   ifs[] = " \t\n\r";			  /* Separators     */
static included_t  *included_files = CC_TNULL(included_t); /* Included files */

cc_conf_status_t cc_conf_read(const char *fn, const cc_conf_kwr_t *kw, void *ud)
{
	int               fd;
	cc_conf_ctx_t    *ct;
	cc_conf_status_t  st;
	included_t       *pi;
	struct stat       fi;

	if(-1 == stat(fn, &fi))
	{
		CC_PROTECT_ERRNO(perror(fn));
		return CC_CONF_ST_SYSTEM_ERROR;
	}
	for(pi = included_files; pi; pi = pi->in_next)
		if(pi->in_ino == fi.st_ino && pi->in_dev == fi.st_dev)
			return CC_CONF_ST_OK;
	if(CC_TNULL(included_t) == (pi = CC_TALLOC(included_t, 1)))
	{
		CC_PROTECT_ERRNO(cc_printf_err("cc_conf_read: Cannot allocate %lu bytes", sizeof(included_t)));
		return CC_CONF_ST_SYSTEM_ERROR;
	}
	pi->in_next    = included_files;
	pi->in_dev     = fi.st_dev;
	pi->in_ino     = fi.st_ino;
	included_files = pi;

	if(-1 == (fd = open(fn, O_RDONLY)))
	{
		CC_PROTECT_ERRNO(perror(fn));
		return CC_CONF_ST_SYSTEM_ERROR;
	}

	if(CC_CONF_ST_OK != (st = context_create(fd, kw, fn, 0, &ct)))
	{
		CC_PROTECT_ERRNO(close(fd));
		return st;
	}
	st = process_lines(ct, ud);
	CC_PROTECT_ERRNO(
		close(fd);
		context_destroy(ct)
		);
	return st;
}

size_t cc_conf_set_bufsiz(size_t ns)
{
	size_t os = bufsize;
	if(0 != ns)
	{
		size_t nulong = (ns + sizeof(unsigned long) - 1) / sizeof(unsigned long);
		bufsize = nulong * sizeof(unsigned long);
	}
	return os;
}

cc_conf_status_t  cc_conf_verror(CC_CONF_CONTEXT ct, cc_conf_status_t st, const char *fp, va_list ap)
{
	static char errbuf[1024];
	CC_PROTECT_ERRNO(
		(void)vsnprintf(errbuf, sizeof(errbuf), fp, ap);
		cc_printf_err("%s[%lu] - %s: %s", ct->ct_filename, ct->ct_lineno, errbuf, cc_conf_strerror(st));
		);
	return st;
}

cc_conf_status_t  cc_conf_error(CC_CONF_CONTEXT ct, cc_conf_status_t st, const char *fm, ...)
{
	cc_conf_status_t rv;
	va_list          ap;
	va_start(ap, fm);
	CC_PROTECT_ERRNO(rv = cc_conf_verror(ct, st, fm, ap));
	va_end(ap);
	return rv;
}

cc_conf_status_t  cc_conf_malformed(CC_CONF_CONTEXT ct, const char *fm, ...)
{
	cc_conf_status_t st;
	va_list          ap;
	va_start(ap, fm);
	CC_PROTECT_ERRNO(st = cc_conf_verror(ct, CC_CONF_ST_SYNTAX_ERROR, fm, ap));
	va_end(ap);
	return st;
}

cc_conf_status_t  cc_conf_perror(CC_CONF_CONTEXT ct, const char *ms)
{
	cc_conf_status_t st;
	CC_PROTECT_ERRNO(st = cc_conf_error(ct, CC_CONF_ST_SYSTEM_ERROR, "%s", ms));
	return st;
}

const char *cc_conf_strerror  (cc_conf_status_t st)
{
	int         en = errno;
	const char *rt;

	switch(st)
	{
	case CC_CONF_ST_SYSTEM_ERROR:		rt = (const char *)strerror(errno);	break;
	case CC_CONF_ST_OK:			rt = "No error";			break;
	case CC_CONF_ST_ERROR:			rt = "Generic error";			break;
	case CC_CONF_ST_SYNTAX_ERROR:		rt = "Syntax error";			break;
	case CC_CONF_ST_VALUE_ERROR:		rt = "Value error";			break;
	case CC_CONF_ST_PREMATURE_END_OF_FILE:	rt = "Premature end of file";		break;
	case CC_CONF_ST_LINE_TOO_LONG:		rt = "Line too long";			break;
	case CC_CONF_ST_TOO_MANY_ARGS:		rt = "Too many args";			break;
	case CC_CONF_ST_INTERNAL:		rt = "Internal error";			break;
	case CC_CONF_ST_END_OF_FILE:		rt = "End of file";			break;
	default:				rt = "Unknown error";			break;
	}
	errno = en;
	return rt;
}

cc_conf_status_t cc_conf_syntaxerr(CC_CONF_CONTEXT ct, const char *fm, ...)
{
	cc_conf_status_t st;
	va_list          ap;
	va_start(ap, fm);
	CC_PROTECT_ERRNO(st = cc_conf_verror(ct, CC_CONF_ST_SYNTAX_ERROR, fm, ap));
	va_end(ap);
	return st;
}

cc_conf_status_t cc_conf_valueerr(CC_CONF_CONTEXT ct, const char *fm, ...)
{
	cc_conf_status_t st;
	va_list          ap;
	va_start(ap, fm);
	CC_PROTECT_ERRNO(st = cc_conf_verror(ct, CC_CONF_ST_VALUE_ERROR, fm, ap));
	va_end(ap);
	return st;
}

cc_conf_status_t  cc_conf_systemerr (CC_CONF_CONTEXT ct, const char *fm, ...)
{
	cc_conf_status_t st;
	va_list          ap;
	va_start(ap, fm);
	CC_PROTECT_ERRNO(st = cc_conf_verror(ct, CC_CONF_ST_SYSTEM_ERROR, fm, ap));
	va_end(ap);
	return st;
}

cc_conf_status_t  cc_conf_enter_blk (CC_CONF_CONTEXT ct, const cc_conf_kwr_t *kw, void *ud)
{
	int                  is_top;
	cc_conf_status_t     status;
	const cc_conf_kwr_t *kwords;

	is_top = ct->ct_is_top;
	kwords = ct->ct_keywords;
	ct->ct_is_top   = 0;
	ct->ct_keywords = kw;
	status = process_lines(ct, ud);
	ct->ct_is_top   = is_top;
	ct->ct_keywords = kwords;
	return status;
}

cc_conf_status_t cc_conf_include(CC_CONF_CONTEXT ct, const cc_conf_kwr_t *kw, void *ud, int ac, char **av)
{
	int                gret;
	glob_t             gval;
	cc_conf_status_t   status;
	size_t             pathc;
	char             **pathv;

	av++, ac--;

	if(ac != 1)
		return cc_conf_malformed(ct, "'%' takes only one argument", kw->kw_name);

	gval.gl_pathc = 0;
	gval.gl_pathv = CC_TNULL(char *);
	gval.gl_offs  = 0;
	if(0 != (gret = glob(*av, GLOB_ERR | GLOB_BRACE | GLOB_TILDE, globing_error, &gval)))
	{
		switch(gret)
		{
		case GLOB_NOSPACE:
			return cc_conf_systemerr(ct, "%s: insufisant memory space", kw->kw_name);
		case GLOB_ABORTED:
			return cc_conf_systemerr(ct, "%s: cannot expand %s", kw->kw_name, *av);
		case GLOB_NOMATCH:
			return cc_conf_systemerr(ct, "%s: no match for %s", kw->kw_name, *av);
		default:
			return cc_conf_error(ct, CC_CONF_ST_INTERNAL, "%s: unknown glob(3) error %d", kw->kw_name, gret);
		}
	}
	for(pathv = gval.gl_pathv, pathc = 0; pathc < gval.gl_pathc; pathc += 1, pathv += 1)
	{
		if(CC_CONF_ST_OK != (status = cc_conf_read(*pathv, ct->ct_keywords, ud)))
		{
			(void)cc_conf_error(ct, status, "%s: cannot process file %s", kw->kw_name, *pathv);
			break;
		}
	}
	CC_PROTECT_ERRNO(globfree(&gval));
	return status;
}

static cc_conf_status_t context_create(int fd, const cc_conf_kwr_t *kw, const char *fn, size_t ln, cc_conf_ctx_t **ct)
{
	char          *ctx_fnam = NULL;
	cc_conf_ctx_t *ctx_new  = NULL;
	size_t         ctx_size = CC_CONF_CTX_HDR_SZ + bufsize;

	if(NULL == (ctx_fnam = cc_strdup(fn)))
	{
		CC_PROTECT_ERRNO(cc_printf_err("context_create: Cannot duplicate string '%s'", fn));
		return CC_CONF_ST_SYSTEM_ERROR;
	}

	if(NULL == (ctx_new = (cc_conf_ctx_t *)cc_malloc(ctx_size)))
	{
		CC_PROTECT_ERRNO(
			cc_printf_err("context_create: Cannot allocate %lu bytes", ctx_size);
			cc_free(ctx_fnam));
		return CC_CONF_ST_SYSTEM_ERROR;
	}
	ctx_new->ct_is_top   = 1;
	ctx_new->ct_filedesc = fd;
	ctx_new->ct_filename = ctx_fnam;
	ctx_new->ct_lineno   = ln;
	ctx_new->ct_keywords = kw;
	ctx_new->ct_bufpos   = (char *)ctx_new->ct_buffer;
	ctx_new->ct_buffre   = (char *)ctx_new->ct_buffer;
	*ct = ctx_new;
	return CC_CONF_ST_OK;
}

static void context_destroy(cc_conf_ctx_t *ct)
{
	const char *fn = ct->ct_filename;
	cc_free((void *)ct);
	cc_free((void *)fn);
	return;
}

static int globing_error(const char *epath, int eerrno)
{
	CC_PROTECT_ERRNO(errno = eerrno; perror(epath));
	return 1;
}

cc_conf_status_t process_lines(cc_conf_ctx_t *ct, void *ud)
{
	int                     argc;
	char                  **argv;
	char                   *line;
	char                   *rest;
	cc_conf_status_t        status;
	const cc_conf_kwr_t    *keyword;

	while(CC_CONF_ST_OK == (status = read_line(ct, &line)))
	{
		for(; *line && isblank(*line); line += 1);
		if('\0' == *line || '#' == *line)
			continue;
		if(CC_CONF_ST_OK != (status = split_line(line, &argv, &argc, &rest)))
			return cc_conf_error(ct, status, "%s", rest);
		for(keyword = ct->ct_keywords; keyword->kw_name; keyword += 1)
			if(0 == strcasecmp(keyword->kw_name, *argv))
				goto found;
		return cc_conf_syntaxerr(ct, "Unknown keyword '%s'", *argv);
	found:
		if(CC_CONF_KW_FUNC_END == keyword->kw_func)
		{
			if(ct->ct_is_top)
				return cc_conf_error(ct, CC_CONF_ST_SYNTAX_ERROR, "Misplaced '%s' directive", keyword->kw_name);
			return CC_CONF_ST_OK;
		}
		if(CC_CONF_ST_OK != (status = keyword->kw_func(ct, keyword, ud, argc, argv)))
			return status;
	}
	if(CC_CONF_ST_END_OF_FILE != status)
		return cc_conf_error(ct, status, "");
	if(0 == ct->ct_is_top)
		return cc_conf_error(ct, CC_CONF_ST_PREMATURE_END_OF_FILE, "%s", ct->ct_filename);
	return CC_CONF_ST_OK;
}

static cc_conf_status_t read_line(cc_conf_ctx_t *ct, char **li)
{
	cc_conf_status_t status;
	if(CC_CONF_ST_OK != read_line0(ct, li))
		return CC_CONF_ST_OK;
	if(CC_CONF_ST_OK != (status = refill_buffer(ct)))
	{
		if(CC_CONF_ST_END_OF_FILE == status && ct->ct_bufpos < ct->ct_buffre)
		{
			*li = ct->ct_bufpos;
			return CC_CONF_ST_OK;
		}
		return status;
	}
	return read_line0(ct, li);
}

static cc_conf_status_t read_line0(cc_conf_ctx_t *ct, char **li)
{
	char *rval;
	char *cpos;
	char *epos;

	epos = ct->ct_buffre;
	for(rval = ct->ct_bufpos; rval < epos && isspace(*rval) && *rval != '\n'; rval += 1);
	for(cpos = rval; cpos < epos; cpos += 1)
	{
		if(*cpos == '\n')
		{
			*(cpos) = '\0';
			ct->ct_bufpos  = cpos + 1;
			ct->ct_lineno += 1;
			while(cpos > rval && isspace(*cpos)) *(cpos--) = '\0';
			*li = rval;
			return CC_CONF_ST_OK;
		}
	}
	return CC_CONF_ST_LINE_TOO_LONG;
}

static cc_conf_status_t refill_buffer  (cc_conf_ctx_t *ct)
{
	size_t           nbytes;
	ssize_t          nread;
	cc_conf_status_t status;

	status = CC_CONF_ST_OK;

	if(ct->ct_bufpos < ct->ct_buffre)
	{
		size_t used = ct->ct_buffre - ct->ct_bufpos;
		(void)memcpy(ct->ct_buffer, ct->ct_bufpos, used);
		nbytes = bufsize - used;
		ct->ct_bufpos = (char *)ct->ct_buffer;
		ct->ct_buffre = ct->ct_bufpos + used;
	}
	else
	{
		ct->ct_bufpos = (char *)ct->ct_buffer;
		ct->ct_buffre = (char *)ct->ct_buffer;
		nbytes = bufsize;
	}
	nread = read(ct->ct_filedesc, ct->ct_buffre, nbytes);
	switch(nread)
	{
	case  0:
		status = CC_CONF_ST_END_OF_FILE;
		break;
	case -1:
		status = CC_CONF_ST_SYSTEM_ERROR;
		break;
	default:
		ct->ct_buffre += nread;
		*(ct->ct_buffre) = '\0';
	}
	return status;
}

static cc_conf_status_t split_line(char *line, char ***argv, int *argc, char **rest)
{
	static char *args[CC_CONF_MAXARGS + 1];

	char    *li;
	char    *re;
	char   **ca;
	char    *na;
	size_t   nb;

	(void)memset(args, 0, sizeof(args));
	for(li = na = line, ca = args, nb = 0; na && nb < CC_CONF_MAXARGS; li = CC_TNULL(char), nb += 1)
	{
		na = strtok_r(li, ifs, &re);
		if(CC_TNULL(char) == na)
			break;
		*(ca++) = na;
	}
	*(ca++) = CC_TNULL(char);
	if(rest)
		*rest = re;
	*argv = args;
	*argc = nb;
	return (re && *re != '\0') ? CC_CONF_ST_TOO_MANY_ARGS : CC_CONF_ST_OK;
}
