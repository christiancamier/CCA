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
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <CCA/confirm.h>
#include <CCA/fmt.h>
#include <CCA/options.h>
#include <CCA/path.h>
#include <CCA/paranoia.h>

#include "erase_internal.h"

#define VERSION             "1.0"
#define DEFAULT_METHOD	    "default"

static int  process_directory(erase_method_t *, const char *);
static int  process_file     (erase_method_t *, const char *);
static void list_methods(int);
static void set_paranoia_level(const char *);
static char *make_default_definitions(const char *);

static void cb_file_start(const char *);
static void cb_file_end(const char *);
static void cb_method_start(const char *, const erase_method_t *);
static void cb_method_end(const char *, const erase_method_t *);
static void cb_pass_start(const char *, const erase_method_t *, uint64_t);
static void cb_pass_end(const char *, const erase_method_t *, uint64_t);
static void cb_block_start(const char *, const erase_method_t *, uint64_t, uint64_t , uint64_t);
static void cb_block_end(const char *, const erase_method_t *, uint64_t, uint64_t , uint64_t);

extern int main(int, char **);

static int options_flgs = 0;
#define FLAG_DEBUG		(int)(1 <<  0)
#define FLAG_VERBOSE		(int)(1 <<  1)
#define FLAG_RECURSIVE		(int)(1 <<  2)
#define FLAG_FORCE		(int)(1 <<  3)
#define FLAG_INTERACTIVE	(int)(1 <<  4)
#define FLAG_SIMULATION		(int)(1 <<  5)
#define FLAG_OUTISTTY		(int)(1 << 14)
#define FLAG_LISTMETHODS	(int)(1 << 15)

#define IS_DEBUG	((options_flgs & FLAG_DEBUG      ) == FLAG_DEBUG      )
#define IS_VERBOSE	((options_flgs & FLAG_VERBOSE    ) == FLAG_VERBOSE    )
#define IS_FORCE	((options_flgs & FLAG_FORCE      ) == FLAG_FORCE      )
#define IS_RECURSIVE	((options_flgs & FLAG_RECURSIVE  ) == FLAG_RECURSIVE  )
#define IS_INTERACTIVE	((options_flgs & FLAG_INTERACTIVE) == FLAG_INTERACTIVE)
#define IS_SIMULATION	((options_flgs & FLAG_SIMULATION ) == FLAG_SIMULATION )
#define IS_OUTISTTY	((options_flgs & FLAG_OUTISTTY   ) == FLAG_OUTISTTY   )
#define IS_LISTMETHODS	((options_flgs & FLAG_LISTMETHODS) == FLAG_LISTMETHODS)

static erase_pass_t default_pass_0 = { .ep_refcnt = ERASE_REFCNT_STATIC, .ep_mode = ERASE_MODE_ZERO };
static erase_pass_t default_pass_1 = { .ep_refcnt = ERASE_REFCNT_STATIC, .ep_mode = ERASE_MODE_ONE  };
static erase_pass_t *default_passes[] = {
	&default_pass_1,
	&default_pass_0,
	&default_pass_1
};

static erase_method_t default_method = {
	.em_marker	= 0UL,
	.em_refcnt	= ERASE_REFCNT_STATIC,
	.em_name	= DEFAULT_METHOD,
	.em_desc	= "Overwrite the file 3 times with varying bit patterns (built-ins)",
	.em_nslots	= sizeof(default_passes) / sizeof(default_passes[0]),
	.em_npass	= sizeof(default_passes) / sizeof(default_passes[0]),
	.em_passes	= default_passes
};

static const char *usage =
	"CC %P %V, secure data removal tool\n\n"
	"Usage: %p [options] file...\n"
	"\n"
	"Options:\n%O\n";

static const char *version =
	"CC %P %V\n"
	"\n"
	"Copyright (c) 2010\n"
	"\tChristian CAMIER <chcamier@free.fr>\n"
	"\n"
	"Permission to use, copy, modify, and distribute this software for any\n"
	"purpose with or without fee is hereby granted, provided that the above\n"
	"copyright notice and this permission notice appear in all copies.\n"
	"\n"
	"THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES\n"
	"WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF\n"
	"MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR\n"
	"ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES\n"
	"WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN\n"
	"ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF\n"
	"OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\n";

static struct cc_option options_defs[] = {
	CC_OPT_ENTRY(
		'd', 'd', "debug",
		CC_OPTARG_NONE,
		&options_flgs, FLAG_DEBUG | FLAG_VERBOSE, CC_OPTARG_OPEOR,
		"display debug informations"),
	CC_OPT_ENTRY(
		'D', 'D', "definition",
		CC_OPTARG_REQUIRED,
		CC_TNULL(int), 0, CC_OPTARG_OPENONE,
		"methods definition files"),
	CC_OPT_ENTRY(
		0, 'f', "force",
		CC_OPTARG_NONE,
		&options_flgs, FLAG_FORCE, CC_OPTARG_OPEOR,
		"ignore nonexistent files, never prompt"),
	CC_OPT_ENTRY(
		'h', 'h', "help",
		CC_OPTARG_NONE,
		CC_TNULL(int), 0, CC_OPTARG_NONE,
		"display this help"),
	CC_OPT_ENTRY(
		0, 'i', "interactive",
		CC_OPTARG_REQUIRED,
		&options_flgs, FLAG_INTERACTIVE, CC_OPTARG_OPEOR,
		"ignore nonexistent files, never prompt"),
	CC_OPT_ENTRY(
		'm', 'm', "method",
		CC_OPTARG_REQUIRED,
		CC_TNULL(int), 0, CC_OPTARG_NONE,
		"erase method to be applyed (default " DEFAULT_METHOD ")"),
	CC_OPT_ENTRY(
		0, 'M', "methods",
		CC_OPTARG_NONE,
		&options_flgs, FLAG_LISTMETHODS, CC_OPTARG_OPEOR,
		"list availlable erase methods"),
	CC_OPT_ENTRY(
		'P', 'P', "paranoia",
		CC_OPTARG_REQUIRED,
		NULL, 0, CC_OPTARG_OPENONE,
		"Memory management paranoia level (none, low, medium, hight)"),
	CC_OPT_ENTRY(
		0, 'r', "recursive",
		CC_OPTARG_NONE,
		&options_flgs, FLAG_RECURSIVE, CC_OPTARG_OPEOR,
		"erase directories and their contents recursively"),
	CC_OPT_ENTRY(
		's', 's', "simulation",
		CC_OPTARG_NONE,
		&options_flgs, FLAG_SIMULATION, CC_OPTARG_OPEOR,
		"simulation mode"),
	CC_OPT_ENTRY(
		'v', 'v', "verbose",
		CC_OPTARG_NONE,
		&options_flgs, FLAG_VERBOSE, CC_OPTARG_OPEOR,
		"explain what is being done"),
	CC_OPT_ENTRY(
		'V', 'V', "version",
		CC_OPTARG_NONE,
		CC_TNULL(int), 0, CC_OPTARG_OPENONE,
		"display programm version"),
};

struct paranoia_level {
	const char *name;
	int         lvel;
};

static struct paranoia_level paranoia_levels[] = {
	{ .name = "none",   .lvel = CC_PARANOIA_NONE   },
	{ .name = "low",    .lvel = CC_PARANOIA_LOW    },
	{ .name = "medium", .lvel = CC_PARANOIA_MEDIUM },
	{ .name = "high",   .lvel = CC_PARANOIA_HIGH   }
};

static char    indentation_buffer[81];
static char   *indentation_lstptr = indentation_buffer;
static size_t  indentation_bnfree = sizeof(indentation_buffer) - 1;

int main(int argc, char **argv)
{
	const char     *definition_file;
	const char     *method_name;
	erase_method_t *method;
	erase_status_t  status;

	definition_file = make_default_definitions(*argv);
	method_name     = DEFAULT_METHOD;

	erase_initialize();
	//erase_option_set_debug(1);
	erase_method_register(&default_method);
	cc_opts_prepare(*argv, VERSION, usage, options_defs, CC_ARRAY_COUNT(options_defs));
	argc -= 1, argv += 1;
	{
		char *aa;
		int   to;

		aa = CC_TNULL(char);
		while(0 != (to = cc_opts_next(&argc, &argv, &aa)))
		{
			switch(to)
			{
			case 'd': erase_option_set_debug(1);		break;
			case 'D': definition_file = (const char *)aa;	break;
			case 'h': cc_opts_usage(0);			break;
			case 'm': method_name     = (const char *)aa;	break;
			case 'p': set_paranoia_level(aa);		break;
			case 's': erase_option_set_simulation(1);	break;
			case 'v': erase_option_set_verbose(1);		break;
			case 'V': cc_opts_version(version);		break;
			default:  cc_opts_usage(200);			break;
			}
		}
	}
	if(argc < 1 && !IS_LISTMETHODS)
		cc_opts_usage(201);

	if(ERA_ST_OK != (status = erase_definition_read_from_file(definition_file)))
	{
		erase_print_error(status, "%s", definition_file);
		exit(202);
	}
		
	if(IS_LISTMETHODS)
		list_methods(0);
	
	if(ERA_ST_OK != (status = erase_method_find(method_name, &method)))
	{
		erase_print_error(status, "%s", method_name);
		list_methods(203);
	}

	if(IS_VERBOSE)
	{
		(void)memset(indentation_buffer, 0x00, sizeof(indentation_buffer));
		(void)erase_set_cb_file_start  (cb_file_start);
		(void)erase_set_cb_file_end    (cb_file_end);
		(void)erase_set_cb_method_start(cb_method_start);
		(void)erase_set_cb_method_end  (cb_method_end);
		(void)erase_set_cb_pass_start  (cb_pass_start);
		(void)erase_set_cb_pass_end    (cb_pass_end);
		(void)erase_set_cb_block_start (cb_block_start);
		(void)erase_set_cb_block_end   (cb_block_end);
		if(isatty(STDOUT_FILENO))
			options_flgs |= IS_OUTISTTY;
	}

	while(argc > 0)
	{
		argc -= 1;
		if(-1 == process_file(method, *(argv++)) && !IS_FORCE)
			return 1;
	}
	return 0;
}

static erase_status_t display_method(void *stream, erase_method_t *method)
{
	fprintf((FILE *)stream, "- %s: %s\n", method->em_name, method->em_desc);
	return ERA_ST_OK;
}

static void list_methods(int status)
{
	FILE *stream = status ? stderr : stdout;
	fprintf(stream, "Allowed methods are:\n");
	(void)erase_method_walk((void *)stream, display_method);
	fflush(stream);
	exit(status);
}

static int process_directory(erase_method_t *method, const char *dirname)
{
	int             i;
	struct dirent **p;
	size_t          dirnamsz;
	int             namelssz;
	struct dirent **namelist;

	if(!IS_RECURSIVE)
	{
		fprintf(stderr, "%s is a directory\n", dirname);
		return -1;
	}

	erase_debug("scaning directory %s", dirname);
	if(-1 == (namelssz = scandir(dirname, &namelist, NULL, alphasort)))
	{
		perror(dirname);
		return -1;
	}
	erase_debug("%d entries found", namelssz);
	erase_info("Entering directory %s", dirname);

	dirnamsz = strlen(dirname);
	for(i = 0, p = namelist; i < namelssz; i += 1, p += 1)
	{
		size_t          filenamsz;
		char           *filename;
		erase_status_t  status;

		filenamsz = dirnamsz + strlen((*p)->d_name) + sizeof("//");

		if(ERA_ST_OK != (status = _erase_memory_alloc(namelssz, (void **)&filename)))
		{
			erase_print_error(status, "%s/%s", dirname, (*p)->d_name);
			return -1;
		}

		snprintf(filename, filenamsz, "%s/%s", filename, (*p)->d_name);
		if(-1 == process_file(method, filename) && !IS_FORCE)
		{
			(void)_erase_memory_free(filename);
			free(namelist);
			return -1;
		}
		(void)_erase_memory_free(filename);
	}
	free(namelist);
	return 0;
}

static int process_file(erase_method_t *method, const char *filename)
{
	int do_crush;
	do_crush = IS_FORCE;
	if(cc_is_directory(filename))
		return process_directory(method, filename);
	if(!do_crush &&
	   (nlink_t)1 < cc_path_nlink(filename) &&
	   !(do_crush = cc_confirm("%s has multiple references.\nWould you like to crush it ?", filename)))
		return 0;
	if(!do_crush &&
	   IS_INTERACTIVE &&
	   !(do_crush = cc_confirm("Would you like to crush %s ?", filename)))
		return 0;

	/* Something to do here */
	return -1;
}

static void set_paranoia_level(const char *level)
{
	struct paranoia_level *pl;
	size_t                 nl;

	for(pl = paranoia_levels, nl = 0; nl < CC_ARRAY_COUNT(paranoia_levels); pl += 1, nl += 1)
		if(0 == strcmp(pl->name, level))
		{
			(void)cc_paranoia_set_level(pl->lvel);
			return;
		}
	cc_opts_usage(202);
}

static char *make_default_definitions(const char *arg0)
{
	size_t max_path_len = cc_path_path_max();
	{
		char    path_name[max_path_len + 1];
		char   *p;
		size_t  s;

		if(NULL == realpath(arg0, path_name))
		{
			perror(arg0);
			exit(1);
		}

		for(p = path_name + strlen(path_name) - 1; p > path_name && *p != '/'; p -= 1);
		if(p == path_name) goto option2;
		for(p -= 1; p > path_name && *p != '/'; p -= 1);
		if(p == path_name) goto option2;
		s = max_path_len - (p - path_name);
		if(14 != cc_fmt_string(&p, &s, "/etc/erase.def"))    goto option2;
		if( 0 == cc_fmt_char(&p, &s, '\0'))                  goto option2;
		if(-1 == access(path_name, R_OK) && ENOENT == errno) goto option2;
		if(CC_TNULL(char) == (p = cc_strdup(path_name)))     goto option2;
		return p;
	}
option2:
	return SYSCONFDIR "/erase.def";
}

static void cb_file_start(const char *filename)
{
	fprintf(stdout, "Erasing file %s\n", filename);
	return;
}

static void cb_file_end(const char *filename)
{
	fprintf(stdout, "%s: done\n", filename);
	return;
}

static void cb_method_start(const char *filename, const erase_method_t *method)
{
	(void)filename;
	fprintf(stdout, "%sApplying method %s\n", indentation_buffer, method->em_name);
	(void)cc_fmt_char(&indentation_lstptr, &indentation_bnfree, ' ');
	return;
}

static void cb_method_end(const char *filename, const erase_method_t *method)
{
	(void)filename;
	(void)method;
	if(indentation_lstptr > indentation_buffer)
	{
		*indentation_lstptr = '\0';
		indentation_lstptr -= 1;
		indentation_bnfree += 1;
	}
	return;
}

static void cb_pass_start(const char *filename, const erase_method_t *method, uint64_t passno)
{
	static const char digits[] = {
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'
	};

	char           desc_buff[32];
	char          *desc_bptr = desc_buff;
	size_t         desc_size = sizeof(desc_buff) - 1;
	erase_pass_t  *pass = *(method->em_passes + passno);
	unsigned char *p;
	size_t        n;

	(void)filename;
	(void)method;

	desc_buff[desc_size] = '\0';
	switch(ERASE_MODE_CODE(pass->ep_mode))
	{
	case ERASE_MODE_BYTE_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Byte ");
		(void)cc_fmt_char(&desc_bptr, &desc_size, digits[(ERASE_MODE_ARG(pass->ep_mode) >> 4) && 0x0F]);
		(void)cc_fmt_char(&desc_bptr, &desc_size, digits[(ERASE_MODE_ARG(pass->ep_mode) >> 0) && 0x0F]);
		break;
	case ERASE_MODE_RNDBYTE_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Random byte ");
		if(ERASE_MODE_ARG(pass->ep_mode)) (void)cc_fmt_string(&desc_bptr, &desc_size, " each");
		else                              (void)cc_fmt_string(&desc_bptr, &desc_size, " once");
		break;
	case ERASE_MODE_RANDOM_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Random buffer ");
		if(ERASE_MODE_ARG(pass->ep_mode)) (void)cc_fmt_string(&desc_bptr, &desc_size, " each");
		else                              (void)cc_fmt_string(&desc_bptr, &desc_size, " once");
		break;
	case ERASE_MODE_PATTERN_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Pattern ");
		for(p = pass->ep_pattern_data, n = pass->ep_pattern_size; n > 0; p += 1, n -= 1)
		{
			(void)cc_fmt_char(&desc_bptr, &desc_size, digits[((*p) >> 4) && 0x0F]);
			(void)cc_fmt_char(&desc_bptr, &desc_size, digits[((*p) >> 0) && 0x0F]);
		}
		break;
	case ERASE_MODE_INVERT_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Bit inversion");
		break;
	case ERASE_MODE_BIT_LROTATE_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Bit left rotation ");
		(void)cc_fmt_uint(&desc_bptr, &desc_size, (unsigned int)ERASE_MODE_ARG(pass->ep_mode), 10);
		break;
	case ERASE_MODE_BIT_RROTATE_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Bit right rotation ");
		(void)cc_fmt_uint(&desc_bptr, &desc_size, (unsigned int)ERASE_MODE_ARG(pass->ep_mode), 10);
		break;
	case ERASE_MODE_BYTE_LROTATE_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Byte left rotation ");
		(void)cc_fmt_uint(&desc_bptr, &desc_size, (unsigned int)ERASE_MODE_ARG(pass->ep_mode), 10);
		break;
	case ERASE_MODE_BYTE_RROTATE_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Byte right rotation ");
		(void)cc_fmt_uint(&desc_bptr, &desc_size, (unsigned int)ERASE_MODE_ARG(pass->ep_mode), 10);
		break;
	case ERASE_MODE_METHOD_CODE:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "Method");
		(void)cc_fmt_string(&desc_bptr, &desc_size, pass->ep_method_def->em_name);
		break;
	default:
		(void)cc_fmt_string(&desc_bptr, &desc_size, "????");
		break;
	}
	(void)cc_fmt_char(&desc_bptr, &desc_size, '\0');
	fprintf(stdout,"%spass #%3" I64F "u (%s)\n", indentation_buffer, passno, desc_buff);
	return;
}

static void cb_pass_end(const char *filename, const erase_method_t *method, uint64_t passno)
{
	(void)filename;
	(void)method;
	(void)passno;
	fprintf(stdout, "\n");
	return;
}

static void cb_block_start(const char *fn, const erase_method_t *m, uint64_t p, uint64_t c, uint64_t n)
{
	(void)fn;
	(void)m;
	(void)p;
	if(IS_OUTISTTY)
	{
		if(n != 0) fprintf(stdout, "%s*Processing block %" I64F "u/%" I64F "u\r", indentation_buffer, c, n);
		else       fprintf(stdout, "%s*Processing block %" I64F "u\r",            indentation_buffer, c);
	}
	return;
}

static void cb_block_end(const char *fn, const erase_method_t *m, uint64_t p, uint64_t c, uint64_t n)
{
	(void)fn;
	(void)m;
	(void)p;
	(void)c;
	(void)n;
	if(IS_OUTISTTY)
		fprintf(stdout, "%s \r", indentation_buffer);
	return;
}
