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

#ifndef __CC_LOG_INTERNAL_H__
#define __CC_LOG_INTERNAL_H__

#include <CCA/log.h>

struct cc_log_code_st {
	const char *c_name;
	size_t      c_nlen;
	int         c_val;
};

#define LOC_CODE_ENTRY(a, b)	{ a, sizeof(a) - 1, b }

extern struct cc_log_code_st cc_log_priorities_tabl[];
extern size_t                cc_log_priorities_nent;

struct cc_log_driver_st {
	struct cc_log_driver_st  *ld_next;
	const  char              *ld_name;
	void                    (*ld_open  )(void);
	void                    (*ld_close )(void);
	void                    (*ld_write )(int, const char *, va_list);
	int                     (*ld_config)(const char *, const char *);
	void                    (*ld_reinit)(void);
};

extern size_t cc_log_format_message(char *, size_t, int, const char *, va_list);
extern void   cc_log_register_driver(struct cc_log_driver_st *);

extern const struct cc_log_code_st *cc_log_search_name(const char *, const struct cc_log_code_st *, size_t);
extern const struct cc_log_code_st *cc_log_search_code(int,          const struct cc_log_code_st *, size_t);

#define cc_log_find_name(name, table) cc_log_search_name(name, table, CC_ARRAY_COUNT(table))
#define cc_log_find_code(code, table) cc_log_search_code(code, table, CC_ARRAY_COUNT(table))

#define default_tfm  "%d/%m/%Y %H:%M:%S";
#define default_ide  "CCLog";
#define default_fac   LOG_USER
#define default_lvl   LOG_INFO
#define default_opt   LOG_PID | LOG_CONS


#endif /*! __CC_LOG_INTERNAL_H__ */
