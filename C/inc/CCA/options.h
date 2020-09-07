/*
 * Copyright (c) 2010
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
 * #@ "options.h"
 *	-- CC Utilities: Common programm startup declarations
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.2 (04/01/2008)
 */

#ifndef __CC_OPTIONS_H__
#define __CC_OPTIONS_H__

/*
 * cc_option structure
 * -------------------
 *
 * This data structure is used to define each possible option.
 * Fields are :
 *      opt_tocken : If non zero, the value is returned by cc_opts_next() when option encountered,
 *                   If zero, cc_opts_next() does not return but affect the option variable and continue
 *                   with the next argument.
 *      opt_sname  : Short name (one letter)
 *      opt_lname  : Long name
 *      opt_lnasz  : Long name length
 *      opt_hasarg : Option argument indicator :
 *                   Possible values are :
 *                     - CC_OPTARG_NONE     : No arguments
 *                     - CC_OPTARG_REQUIRED : An argument is required
 *      opt_varptr : Integer variable address (NULL if none)
 *      opt_varval : Value operand for variable operation
 *      opt_varope : Operation ti apply on variable referenced by opt_varptr when operation encountered
 *                   Possible value are :
 *                     - CC_OPTARG_OPENONE : variable is not affected
 *                     - CC_OPTARG_OPESET  : variable ::= value
 *                     - CC_OPTARG_OPEADD  : variable ::= variable + value
 *                     - CC_OPTARG_OPESUB  : variable ::= variable - value
 *                     - CC_OPTARG_OPEAND  : variable ::= variable .AND. value
 *                     - CC_OPTARG_OPEOR   : variable ::= variable .OR.  value
 *                     - CC_OPTARG_OPEXOR  : variable ::= variable .XOR. value
 *	opt_info   : Option information for cc_opts_usage()
 */
struct cc_option {
	int	    opt_tocken;		/* Tocken (value returned by getnextoption */
	int         opt_sname;		/* Short option                            */
	const char *opt_lname;		/* Long option                             */
	size_t      opt_lnasz;		/* Long option size                        */
	int         opt_hasarg;		/* Has argument                            */
#define CC_OPTARG_NONE		0	/* Option has no arguments                 */
#define CC_OPTARG_REQUIRED	1	/* Option require an argument              */
	int        *opt_varptr;		/* Variable pointer (NULL if none)         */
	int         opt_varval;		/* Variable value                          */
	int         opt_varope;		/* Variable operation			   */
#define CC_OPTARG_OPENONE	0	/* No operation                            */
#define CC_OPTARG_OPESET	1	/* Variable <- value			   */
#define CC_OPTARG_OPEADD	2	/* Variable <- variable + value            */
#define CC_OPTARG_OPESUB	3	/* Variable <- variable - value            */
#define CC_OPTARG_OPEAND	4	/* Variable <- variable .AND. value        */
#define CC_OPTARG_OPEOR		5	/* Variable <- variable .OR.  value        */
#define CC_OPTARG_OPEXOR	6	/* Variable <- variable .XOR. value        */
	const char *opt_info;		/* Option information (for usage())        */
};

#define CC_OPT_ENTRY(tocken, sname, lname, hasarg, varptr, varval, varope, info) \
  { tocken, sname, lname, sizeof(lname) - 1, hasarg, varptr, varval, varope, info }

extern       void  cc_opts_prepare(const char *, const char *, const char *, const struct cc_option *, size_t);
extern       void  cc_opts_usage(int status);
extern       void  cc_opts_version(const char *);
extern       int   cc_opts_next(int *, char ***, char **);
extern const char *cc_opts_progname(void);

#endif /*! __CC_OPTIONS_H__*/
