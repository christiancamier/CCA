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
 * #@ "cc_options.c"
 *	-- CC Utilities: Common programm startup procedures
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * Rev : 1.1 (01/22/2008)
 */

/*
 * Changelog:
 * =========
 *
 * 01/21/2008: Rev 1.0 -- Christian CAMIER (chcamier@free.fr)
 *	- Creation
 *
 * 01/22/2008: Rev 1.1 -- Christian CAMIER (chcamier@free.fr)
 *	- M usage_options :
 *	  Added an '=xxx' display for long options with args.
 *	- A exported function "cc_opts_version"
 *
 * 04/01/2008: Rev 1.2 -- Christian CAMIER (chcamier@free.fr)
 *	- Added variable operation (opt_varope) behaviour
 */
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <CCA/options.h>

/* Exported functions */
extern       void  cc_opts_prepare(const char *, const char *, const char *, const struct cc_option *, size_t);
extern       void  cc_opts_usage(int status);
extern       void  cc_opts_version(const char *);
extern       int   cc_opts_next(int *, char ***, char **);
extern const char *cc_opts_progname(void);

/* Local functions */
static void usage_options(FILE *);


static const char             *program_name = (const char             *)NULL;
static const char             *program_vers = (const char             *)NULL;
static const char             *usage_string = (const char             *)NULL;
static const struct cc_option *program_opts = (const struct cc_option *)NULL;
static size_t                  program_nopt = (size_t)0;

/*
 * NAME
 *	cc_opts_prepare
 * SYNOPSIS
 *	#include <sys/types.h>
 *	#include <CCA/options.h>
 *	void cc_opts_prepare(
 *		const char *prg_name,
 *		const char *prg_version,
 *		const char *prg_usage,
 *		const struct cc_option *prg_options,
 *		size_t prg_numopts);
 *
 * DESCRIPTION
 *	This function, called at the beginning of the programm, initialise,
 *	all parametters for the cc_options suite. The arguments are :
 *	prg_name:	Name of the program (usualy argv[0]). If name is a path,
 *			the basename part will be retrieved.
 *	prg_version:	Version of the programm
 *	prg_usage:	Usage string
 *	prg_options:	Array of the options for this programm
 *	prg_numopts:	Number of options in prg_options.
 *
 *	The usage string may contain some specifics sequences (a percent character
 *	followed by a letter) having special meaning. The allowed sequences are:
 *	. %O: The options with there own descriptions,
 *	. %P: The capatilized program name
 *	. %p: The program name
 *	. %V: The program version
 *	. %%: A percent character
 *		
 * RETURN VALUE
 *	None
 */
void cc_opts_prepare(
	const char *prg_name,
	const char *prg_version,
	const char *prg_usage,
	const struct cc_option *prg_options,
	size_t prg_numopts)
{
	register char *ptr;

	/* Store programm name */
	if((char *)NULL != (ptr = strrchr(prg_name, '/'))) ptr = ptr + 1;
	else                                               ptr = (char *)prg_name;
	program_name = (const char *)ptr;

	/* Then program version */
	program_vers = prg_version;

	/* Then program usage */
	usage_string = prg_usage;

	/* Then options */
	program_opts = prg_options;
	program_nopt = prg_numopts;

	return;
}

/*
 * NAME
 *	cc_opts_usage -- Display program usage and exit
 *
 * SYNOPSIS
 *	#include <sys/types.h>
 *	#include <CCA/options.h>
 *	void cc_opts_usage(int status);
 *
 * DESCRIPTION
 *	Display the usage string and the exit with the given status.
 *	If status is not zero, outputs are processed thru the stderr
 *	stream.
 *	If status is zero, outputs are processed thru the stdout stream
 *
 * RETURN VALUE
 *	None
 */
void cc_opts_usage(int status)
{
#define PUTS(S) for(nfoptr = S; *nfoptr; fputc(*(nfoptr++), stream))
	register FILE       *stream;
	register const char *usaptr;
	register const char *nfoptr;

	stream = status ? stderr : stdout;

	fputc('\n', stream);
	usaptr = usage_string;
	while(*usaptr)
	{
		if('%' == *usaptr)
		{
			switch(*(usaptr + 1))
			{
			case 'O':	/* Options */
				usaptr += 1;
				usage_options(stream);
				break;
			case 'P':	/* Capitalized program name */
				fputc(toupper(*program_name), stream);
				PUTS(program_name + 1);
				usaptr += 1;
				break;
			case 'p':	/* Program name */
				PUTS(program_name);
				usaptr += 1;
				break;
			case 'V':	/* Program version */
				PUTS("Ver. ");
				PUTS(program_vers);
				usaptr += 1;
				break;
			case '%':	/* A percent character */
				usaptr += 1;
				/* No break here */
			case '\0':	/* String terminaison */
			default:
				fputc('%', stream);
				break;
			}
		}
		else
		{
			fputc(*usaptr, stream);
		}
		usaptr += 1;
	}
	fputc('\n', stream);
	exit(status);
#undef PUTS
}

/*
 * NAME
 *	cc_opts_version -- Display program version and exit
 *
 * SYNOPSIS
 *	#include <sys/types.h>
 *	#include <CCA/options.h>
 *	void cc_opts_version(const char version_string);
 *
 * DESCRIPTION
 *	Display the given version string and exit with status 0.
 *
 *	The version string may contain some specifics sequences (a percent character
 *	followed by a letter) having special meaning. The allowed sequences are:
 *	. %P: The capatilized program name
 *	. %p: The program name
 *	. %V: The program version
 *	. %%: A percent character
 *
 * RETURN VALUE
 *	None
 */
void cc_opts_version(const char *version_string)
{
#define PUTS(S) for(nfoptr = S; *nfoptr; fputc(*(nfoptr++), stdout))
	register const char             *verptr;
	register const char             *nfoptr;

	fputc('\n', stdout);
	verptr = version_string;
	while(*verptr)
	{
		if('%' == *verptr)
		{
			switch(*(verptr + 1))
			{
			case 'P':	/* Capitalized program name */
				fputc(toupper(*program_name), stdout);
				PUTS(program_name + 1);
				verptr += 1;
				break;
			case 'p':	/* Program name */
				PUTS(program_name);
				verptr += 1;
				break;
			case 'V':	/* Program version */
				PUTS("Ver. ");
				PUTS(program_vers);
				verptr += 1;
				break;
			case '%':	/* A percent character */
				verptr += 1;
				/* No break here */
			case '\0':	/* String terminaison */
			default:
				fputc('%', stdout);
				break;
			}
		}
		else
		{
			fputc(*verptr, stdout);
		}
		verptr += 1;
	}
	fputc('\n', stdout);
	fflush(stdout);
	exit(0);
#undef PUTS
}

/*
 * NAME
 *	cc_opts_next -- Get next option
 *
 * SYNOPSIS
 *	#include <sys/types.h>
 *	#include <CCA/options.h>
 *	int cc_opts_next(int *argc, char ***argv, char **carg);
 *
 * DESCRIPTION
 *	Parse the command-line arguments according to the options
 *	defined using the cc_opts_prepare(3ccr) function.
 *
 * RETURN VALUE
 *	None
 */
int cc_opts_next(int *argc, char ***argv, char **carg)
{
	register int                      ac;
	register char                   **av;
	register char                    *pa;
	register const struct cc_option  *op;
	register size_t                   oc;

	if(0 >= (ac = *argc))
		return 0;

	do
	{
		av = *argv;
		pa = *av;

		if(*pa != '-')
			return 0;

		pa += 1;
		if(*pa == '-')
			goto longopt;

		for(op = program_opts, oc = 0; oc < program_nopt; oc += 1, op += 1)
			if(op->opt_sname == (int)*pa)
				goto shortfnd;
		fprintf(stderr, "Unknown option -%c\n", *pa);
		cc_opts_usage(200);
	shortfnd:
		pa += 1;
		if(op->opt_hasarg)
		{
			if(*pa == '\0')
			{
				if(ac < 2)
					goto missarg;
				ac -= 2, *carg = *(++av), av += 1;
				goto optret2;
			}
			*carg=pa;
			goto optret1;
		}
		if(*pa != '\0')
		{
			*(pa - 1) = '-', *av = pa - 1;
			goto optret2;
		}
		goto optret1;
	longopt:
		pa += 1;
		if(*pa == '\0')			/* '--' case */
		{
			*argc = ac - 1;
			*argv = av + 1;
			return 0;
		}

		for(op = program_opts, oc = 0; oc < program_nopt; oc += 1, op += 1)
			if(0 == strncmp(op->opt_lname, pa, op->opt_lnasz))
			{
				int c;
				c = *(pa + op->opt_lnasz);
				if(c == '\0' || c == '=')
					goto longfnd;
			}
		fprintf(stderr, "Unknown option --%s\n", pa);
		cc_opts_usage(200);
	longfnd:
		pa += op->opt_lnasz;
		if(op->opt_hasarg)
		{
			if(*pa != '=')
				goto missarg;
			*carg = (pa + 1);
			goto optret1;
		}
		if(*pa != '=')
			goto optret1;
		fprintf(stderr, "Misformed option `--%s'\n", op->opt_lname);
		cc_opts_usage(202);
	missarg:
		fprintf(stderr, "Option `-%c, --%s' argument missing\n", op->opt_sname, op->opt_lname);
		cc_opts_usage(201);
	optret1:
		ac -= 1;
		av += 1;
	optret2:
		*argc = ac;
		*argv = av;
		if(op->opt_varptr != (int *)NULL)
			switch(op->opt_varope)
			{
			case CC_OPTARG_OPENONE:						break;
			case CC_OPTARG_OPESET:	*op->opt_varptr  = op->opt_varval;	break;
			case CC_OPTARG_OPEADD:	*op->opt_varptr += op->opt_varval;	break;
			case CC_OPTARG_OPESUB:	*op->opt_varptr -= op->opt_varval;	break;
			case CC_OPTARG_OPEAND:	*op->opt_varptr &= op->opt_varval;	break;
			case CC_OPTARG_OPEOR:	*op->opt_varptr |= op->opt_varval;	break;
			case CC_OPTARG_OPEXOR:	*op->opt_varptr ^= op->opt_varval;	break;
			default:							break;
			}
	}
	while(0 == op->opt_tocken && 0 != ac);
	return op->opt_tocken;
}

const char *
cc_opts_progname(void)
{
	return program_name;
}

static void usage_options(FILE *stream)
{
	register const struct cc_option *optptr;
	register size_t                  optcnt;

	fflush(stream);
	for(optcnt = 0, optptr = program_opts; optcnt < program_nopt; optcnt += 1, optptr += 1)
		fprintf(
			stream,
			" -%c, --%s%s\n\t%s\n",
			optptr->opt_sname,
			optptr->opt_lname,
			optptr->opt_hasarg ? "=xxx" : "",
			optptr->opt_info
			);
	fflush(stream);
	return;
}
