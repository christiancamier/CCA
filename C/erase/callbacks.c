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

#include <stdio.h>
#include <stdlib.h>

#include <cc_util.h>

#include "erase_internal.h"

extern erase_cb_file_t   erase_get_cb_file_start(void);
extern erase_cb_file_t   erase_get_cb_file_end(void);
extern erase_cb_method_t erase_get_cb_method_start(void);
extern erase_cb_method_t erase_get_cb_method_end(void);
extern erase_cb_pass_t   erase_get_cb_pass_start(void);
extern erase_cb_pass_t   erase_get_cb_pass_end(void);
extern erase_cb_block_t  erase_get_cb_block_start(void);
extern erase_cb_block_t  erase_get_cb_block_end(void);
extern erase_cb_msg_t    erase_get_cb_debug(void);
extern erase_cb_msg_t    erase_get_cb_info(void);

extern erase_cb_file_t   erase_set_cb_file_start(erase_cb_file_t);
extern erase_cb_file_t   erase_set_cb_file_end(erase_cb_file_t);
extern erase_cb_method_t erase_set_cb_method_start(erase_cb_method_t);
extern erase_cb_method_t erase_set_cb_method_end(erase_cb_method_t);
extern erase_cb_pass_t   erase_set_cb_pass_start(erase_cb_pass_t);
extern erase_cb_pass_t   erase_set_cb_pass_end(erase_cb_pass_t);
extern erase_cb_block_t  erase_set_cb_block_start(erase_cb_block_t);
extern erase_cb_block_t  erase_set_cb_block_end(erase_cb_block_t);
extern erase_cb_msg_t    erase_set_cb_debug(erase_cb_msg_t);
extern erase_cb_msg_t    erase_set_cb_info(erase_cb_msg_t);

extern erase_cb_file_t   _erase_cb_file_start;
extern erase_cb_file_t   _erase_cb_file_end;
extern erase_cb_method_t _erase_cb_method_start;
extern erase_cb_method_t _erase_cb_method_end;
extern erase_cb_pass_t   _erase_cb_pass_start;
extern erase_cb_pass_t   _erase_cb_pass_end;
extern erase_cb_block_t  _erase_cb_block_start;
extern erase_cb_block_t  _erase_cb_block_end;
extern erase_cb_msg_t    _erase_cb_debug;
extern erase_cb_msg_t    _erase_cb_info;
extern erase_cb_msg_t    _erase_cb_error;

static void default_file_start(const char *);
static void default_file_end(const char *);
static void default_method_start(const char *, const erase_method_t *);
static void default_method_end(const char *, const erase_method_t *);
static void default_pass_start(const char *, const erase_method_t *, uint64_t);
static void default_pass_end(const char *, const erase_method_t *, uint64_t);
static void default_block_start(const char *, const erase_method_t *, uint64_t, uint64_t , uint64_t);
static void default_block_end(const char *, const erase_method_t *, uint64_t, uint64_t , uint64_t);
static void default_debug(const char *);
static void default_info(const char *);
static void default_error(const char *);

erase_cb_file_t   _erase_cb_file_start	 = default_file_start;
erase_cb_file_t   _erase_cb_file_end	 = default_file_end;
erase_cb_method_t _erase_cb_method_start = default_method_start;
erase_cb_method_t _erase_cb_method_end	 = default_method_end;
erase_cb_pass_t   _erase_cb_pass_start	 = default_pass_start;
erase_cb_pass_t   _erase_cb_pass_end	 = default_pass_end;
erase_cb_block_t  _erase_cb_block_start	 = default_block_start;
erase_cb_block_t  _erase_cb_block_end	 = default_block_end;
erase_cb_msg_t    _erase_cb_debug        = default_debug;
erase_cb_msg_t    _erase_cb_info         = default_info;
erase_cb_msg_t    _erase_cb_error        = default_error;

#define IFNULL(V, D) (V) ? (V) : (D)
#define DEFINE_FUNCTION(TYPE, NAME) \
	TYPE erase_get_cb_##NAME(void) \
	{ \
		return _erase_cb_##NAME; \
	} \
	TYPE erase_set_cb_##NAME(TYPE cb) \
	{ \
		TYPE old = _erase_cb_##NAME;	       \
		_erase_cb_##NAME = IFNULL(cb, default_##NAME);	\
		return old;					\
	}

DEFINE_FUNCTION(erase_cb_file_t,   file_start)
DEFINE_FUNCTION(erase_cb_file_t,   file_end)
DEFINE_FUNCTION(erase_cb_method_t, method_start)
DEFINE_FUNCTION(erase_cb_method_t, method_end)
DEFINE_FUNCTION(erase_cb_pass_t,   pass_start)
DEFINE_FUNCTION(erase_cb_pass_t,   pass_end)
DEFINE_FUNCTION(erase_cb_block_t,  block_start)
DEFINE_FUNCTION(erase_cb_block_t,  block_end)
DEFINE_FUNCTION(erase_cb_msg_t,    debug)
DEFINE_FUNCTION(erase_cb_msg_t,    info)

#undef DEFINE_FUNCTION
#undef IFNULL

static void default_file_start(const char *filename)
{
	(void)filename;
	return;
}

static void default_file_end(const char *filename)
{
	(void)filename;
	return;
}

static void default_method_start(const char *filename, const erase_method_t *method)
{
	(void)filename;
	(void)method;
	return;
}

static void default_method_end(const char *filename, const erase_method_t *method)
{
	(void)filename;
	(void)method;
	return;
}

static void default_pass_start(const char *filename, const erase_method_t *method, uint64_t passno)
{
	(void)filename;
	(void)method;
	(void)passno;
	return;
}

static void default_pass_end(const char *filename, const erase_method_t *method, uint64_t passno)
{
	(void)filename;
	(void)method;
	(void)passno;
	return;
}

static void default_block_start(const char *fn, const erase_method_t *m, uint64_t passno, uint64_t cblk, uint64_t nblk)
{
	(void)fn;
	(void)m;
	(void)passno;
	(void)cblk;
	(void)nblk;
	return;
}

static void default_block_end(const char *fn, const erase_method_t *m, uint64_t passno, uint64_t cblk, uint64_t nblk)
{
	(void)fn;
	(void)m;
	(void)passno;
	(void)cblk;
	(void)nblk;
	return;
}

static void default_debug(const char *message)
{
	fprintf(stderr, "DEBUG: %s\n", message);
	return;
}

static void default_info(const char *message)
{
	fprintf(stdout, "INFO: %s\n", message);
	return;
}

static void default_error(const char *message)
{
	fprintf(stderr, "ERROR; %s\n", message);
	return;
}
