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

#include <cc_machdep.h>

#include <CCA/memory.h>
#include <CCA/util.h>

#include <stdint.h>

#include "erase.h"

#ifndef __ERASE_INTERNAL_H__
#define __ERASE_INTERNAL_H__

typedef enum {
	erase_false = (1 == 0),
	erase_true  = (1 == 1)
} erase_bool_t;

typedef struct {
	long           bu_flags;
#define ERA_BUF_INITIALIZED	(1 <<  1)
#define ERA_BUF_RECOMPUTE	(1 <<  2)
	size_t         bu_size;
	union {
		unsigned char  _bu_cdata[1];
		unsigned long  _bu_ldata[1];
	} _bu_data;
#define bu_cdata _bu_data._bu_cdata
#define bu_ldata _bu_data._bu_ldata
} erase_buffer_t;

#define ULONG_BITS	  (8 * sizeof(unsigned long))
#define BUFFER_ROUNDUP(S) (((S) + sizeof(unsigned long) - 1) / sizeof(unsigned long))
#define BUFFER_BYTES(S)	  ((S) * sizeof(unsigned long))
#define BUFFER_HDRSIZE    BUFFER_BYTES(BUFFER_ROUNDUP(CC_STRUCT_OFFSET_OF(erase_buffer_t, _bu_data)))
#define DEF_BUFSIZE 1024

#define ERASE_MODE_METHOD_NAMED		ERASE_MAKE_MODE(ERASE_MODE_METHOD_CODE, 0x00)
#define ERASE_MODE_METHOD_RESOLVED	ERASE_MAKE_MODE(ERASE_MODE_METHOD_CODE, 0xFF)

struct erase_file_st;

typedef struct erase_driver_st {
	struct erase_driver_st  *next;
	const char              *name;
	erase_status_t         (*is_a)        (struct erase_file_st *);
	erase_status_t         (*initialize)  (struct erase_file_st *);
	erase_status_t         (*process_pass)(struct erase_file_st *, erase_method_t *, uint64_t);
	erase_status_t         (*finalize)    (struct erase_file_st *);
} erase_driver_t;

typedef struct erase_file_st {
	char              *fname;
	int                fdesc;
	uint64_t           fsize;
	uint64_t           blkcur;
	uint64_t           blkcnt;
	erase_driver_t    *driver;
	erase_buffer_t    *buffer;
} erase_file_t;

typedef struct erase_warning_st {
	struct erase_warning_st *next;
	const char              *message;
} erase_warning_t;

#define WARNING(M) \
	static erase_warning_t warn_message = { .message = M }; \
	static __attribute__((constructor)) void warn_fct(void) { _erase_warning_add(&warn_message); }

#define ERASE_PASS_INCREMENT	16

extern unsigned long _erase_options;
#define OPTION_VERBOSE			(1 <<  0)
#define OPTION_DEBUG			(1 <<  1)
#define OPTION_BACK_NAME_RESOLUTION	(1 <<  2)
#define OPTION_SIMULATION		(1 <<  3)

#define is_verbose()		  ((_erase_options & OPTION_VERBOSE		) == OPTION_VERBOSE		)
#define is_debug()		  ((_erase_options & OPTION_DEBUG		) == OPTION_DEBUG		)
#define is_simulation()		  ((_erase_options & OPTION_SIMULATION		) == OPTION_SIMULATION		)
#define is_back_name_resolution() ((_erase_options & OPTION_BACK_NAME_RESOLUTION) == OPTION_BACK_NAME_RESOLUTION)

extern char          _erase_io_buffer[];
extern size_t        _erase_io_bufsiz;

/* buffer.c */
extern erase_status_t _erase_buffer_create(size_t, erase_buffer_t **);
extern erase_status_t _erase_buffer_destroy(erase_buffer_t *);
extern erase_status_t _erase_fill_buffer(erase_buffer_t *, erase_pass_t *);
extern erase_status_t _erase_fill_buffer_reset(erase_buffer_t *);

/* callbacks.c */
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
/* driver.c */
extern void           _erase_driver_register(erase_driver_t *);
extern erase_status_t _erase_driver_find    (erase_file_t *);

/* fileop.c */
extern erase_status_t _erase_file_close(erase_file_t *);
extern erase_status_t _erase_file_open (const char *, erase_file_t **);
extern const char    *_erase_file_type(erase_file_t *);

/* memory.c */
extern erase_status_t _erase_memdup	   (const void *, size_t, void **);
extern erase_status_t _erase_memory_alloc  (size_t, void **);
extern erase_status_t _erase_memory_free   (void *);
extern erase_status_t _erase_memory_realloc(void *, size_t, void **);
extern erase_status_t _erase_strdup        (const char *, char **);

/* methods.c */
extern erase_status_t _erase_method_process(erase_file_t *, erase_method_t *);

/* passes.c */
extern erase_status_t _erase_pass_process(erase_file_t *, erase_method_t *, uint64_t);

/* random.c */
extern void          _erase_random_init(void);
extern unsigned long _erase_random     (void);

/* warning.c */
extern void           _erase_warning_add    (erase_warning_t *);
extern void           _erase_warning_display(void);
extern erase_status_t _erase_warning_exist  (void);

#endif /*! __ERASE_INTERNAL_H__*/
