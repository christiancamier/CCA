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

#ifndef __CC_CONFIGURATION_H__
#define __CC_CONFIGURATION_H__

struct cc_conf_ctx_st;
struct cc_conf_kwr_st;

#ifndef __CC_CONFIGURATION_INTERNAL__
typedef void *CC_CONF_CONTEXT
#endif

typedef enum cc_conf_status_en {
	CC_CONF_ST_SYSTEM_ERROR			= -1, /* System error                               */
	CC_CONF_ST_OK				=  0, /* OK                                         */
	CC_CONF_ST_ERROR,			/* Returned by cc_conf_read in errors occurs        */
	CC_CONF_ST_SYNTAX_ERROR,		/* Syntax error as occured                          */
	CC_CONF_ST_VALUE_ERROR,			/* Bad argument                                     */
	CC_CONF_ST_PREMATURE_END_OF_FILE,	/* Premature end of file error                      */
	CC_CONF_ST_LINE_TOO_LONG,		/* Configuration line too long                      */
	CC_CONF_ST_TOO_MANY_ARGS,		/* Too many args                                    */
	CC_CONF_ST_INTERNAL,			/* Internal error                                   */
	CC_CONF_ST_END_OF_FILE,		        /* End of file occured                              */
} cc_conf_status_t;

typedef cc_conf_status_t (*cc_conf_kwr_func_t)(struct cc_conf_ctx_st *, const struct cc_conf_kwr_st *, void *, int, char **);
#define CC_CONF_MAXARGS	32

#define CC_CONF_CTX_HDR_SZ (size_t)(((struct cc_conf_ctx_st *)0)->ct_buffer)

typedef struct cc_conf_kwr_st {
	const char          *kw_name;		/* Keyword name        */
	long                 kw_info;		/* User informations   */
	cc_conf_kwr_func_t   kw_func;		/* Associated function */
} cc_conf_kwr_t;

#define CC_CONF_KW_FUNC_END  (cc_conf_kwr_func_t)-1

extern cc_conf_status_t cc_conf_read      (const char *, const cc_conf_kwr_t *, void *);
extern size_t           cc_conf_set_bufsiz(size_t);

/*
 * Error displaying functions
 */
#ifdef _STDARG_H
extern cc_conf_status_t cc_conf_verror    (CC_CONF_CONTEXT, cc_conf_status_t, const char *, va_list);
#endif

extern cc_conf_status_t cc_conf_error     (CC_CONF_CONTEXT, cc_conf_status_t, const char *, ...);
extern cc_conf_status_t cc_conf_misformed (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t cc_conf_perror    (CC_CONF_CONTEXT, const char *);
extern cc_conf_status_t cc_conf_syntaxerr (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t cc_conf_valueerr  (CC_CONF_CONTEXT, const char *, ...);
extern cc_conf_status_t cc_conf_systemerr (CC_CONF_CONTEXT, const char *, ...);

/*
 * Helper functions
 */
extern cc_conf_status_t cc_conf_enter_blk (CC_CONF_CONTEXT, const cc_conf_kwr_t *, void *);
extern cc_conf_status_t cc_conf_include   (CC_CONF_CONTEXT, const cc_conf_kwr_t *, void *, int, char **);

extern void (*cc_conf_set_errdisp(void (*)(const char *, ...)))(const char *, ...);

#endif  /* !__CC_CONFIGURATION_H__ */
