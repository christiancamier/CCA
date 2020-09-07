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

#ifndef __ERASE_H__
#define __ERASE_H__

typedef enum erase_status_en {
	ERA_ST_SYSTEM_ERROR			= -1,
	ERA_ST_OK				=  0,
	ERA_ST_WARNING,
	ERA_ST_ITER_STOP,
	ERA_ST_DONT_MATCH,
	ERA_ST_UNKNOWN_METHOD,
	ERA_ST_METHOD_EXISTS,
	ERA_ST_METHOD_INCOMPLETE,
	ERA_ST_NOT_IMPLEMENTED,
	ERA_ST_IS_STATIC,
	ERA_ST_CYCLE_DETECTED,
	ERA_ST_LINE_TOO_LONG,
	ERA_ST_SYNTAX_ERROR,
	ERA_ST_VALUE_ERROR,
	ERA_ST_PREMATURE_END_OF_FILE,
	ERA_ST_END_OF_FILE,
	ERA_ST_INTERNAL,
} erase_status_t;

/*
 * erase_pass_t:
 * =============
 *
 * Members:
 * --------
 *
 * - ep_refcnt:
 *
 * - ep_mode:
 *
 * - ep_pattern_data:
 *
 * - ep_pattern_size:
 *
 * - ep_method_name:
 *
 * - ep_method_def:
 */

typedef struct erase_pass_st {
	unsigned long ep_refcnt;
	unsigned long ep_mode;
	union {
		struct {
			size_t         size;
			unsigned char *data;
		} _ep_pattern;
		union {
			char                   *name;
			struct erase_method_st *definition;
		} _ep_method;
	} _ep_data;
#define ep_pattern_data _ep_data._ep_pattern.data
#define ep_pattern_size _ep_data._ep_pattern.size
#define ep_method_name  _ep_data._ep_method.name
#define ep_method_def   _ep_data._ep_method.definition
} erase_pass_t;

/*
 * Erase modes definitions:
 * ========================
 */
#define ERASE_MODE_CODE(M)		(((M) & 0xFF00) >> 8)
#define ERASE_MODE_ARG(M)		((M) & 0x00FF)

#define ERASE_MODE_BYTE_CODE		0x00
#define ERASE_MODE_RNDBYTE_CODE		0x01
#define ERASE_MODE_RANDOM_CODE		0X02
#define ERASE_MODE_PATTERN_CODE		0x03
#define ERASE_MODE_INVERT_CODE		0x04
#define ERASE_MODE_BIT_LROTATE_CODE	0x05
#define ERASE_MODE_BIT_RROTATE_CODE	0x06
#define ERASE_MODE_BYTE_LROTATE_CODE	0x07
#define ERASE_MODE_BYTE_RROTATE_CODE	0x08
#define ERASE_MODE_METHOD_CODE		0xFF

#define ERASE_MAKE_MODE(code, arg)	(unsigned long)((((code) & 0xFF) << 8) | ((arg) & 0xFF))

#define ERASE_MODE_BYTE(B)		ERASE_MAKE_MODE(ERASE_MODE_BYTE_CODE, (B))
#define ERASE_MODE_ZERO			ERASE_MAKE_MODE(ERASE_MODE_BYTE_CODE, 0x00)
#define ERASE_MODE_ONE			ERASE_MAKE_MODE(ERASE_MODE_BYTE_CODE, 0xFF)
#define ERASE_MODE_RNDBYTE(each)	ERASE_MAKE_MODE(ERASE_MODE_RNDBYTE_CODE, (each) ? 0xFF : 0x00)
#define ERASE_MODE_RNDBYTE_ONCE		ERASE_MAKE_MODE(ERASE_MODE_RNDBYTE_CODE, 0x00)
#define ERASE_MODE_RNDBYTE_EACH		ERASE_MAKE_MODE(ERASE_MODE_RNDBYTE_CODE, 0xFF)
#define ERASE_MODE_RANDOM(each)		ERASE_MAKE_MODE(ERASE_MODE_RANDOM_CODE, (each) ? 0xFF : 0x00)
#define ERASE_MODE_RANDOM_ONCE		ERASE_MAKE_MODE(ERASE_MODE_RANDOM_CODE, 0x00)
#define ERASE_MODE_RANDOM_EACH		ERASE_MAKE_MODE(ERASE_MODE_RANDOM_CODE, 0xFF)
#define ERASE_MODE_PATTERN		ERASE_MAKE_MODE(ERASE_MODE_PATTERN_CODE, 0x00)
#define ERASE_MODE_INVERT		ERASE_MAKE_MODE(ERASE_MODE_INVERT_CODE, 0x00)
#define ERASE_MODE_BIT_ROTATE(N)	ERASE_MAKE_MODE(ERASE_MODE_BIT_ROTATE_CODE, (N))
#define ERASE_MODE_BYTE_ROTATE_CODE(N)	ERASE_MAKE_MODE(ERASE_MODE_BYTE_ROTATE_CODE, (N))
#define ERASE_MODE_METHOD		ERASE_MAKE_MODE(ERASE_MODE_METHOD_CODE, 0x00)

/*
 * erase_method_t:
 * ===============
 *
 * Members:
 * --------
 *
 * - em_refcnt:
 *
 * - em_marker:
 *
 * - em_name:
 *
 * - em_desc:
 *
 * - em_nslots:
 *
 * - em_npass:
 *
 * - em_passes:
 */

typedef struct erase_method_st {
	unsigned long   em_refcnt; /* Internal use marker */
	unsigned long   em_marker; /* Internal use marker */
	char           *em_name;   /* Name */
	char           *em_desc;   /* Description */
	size_t          em_nslots; /* Number of allocated passes slots */
	size_t          em_npass;  /* Number of defined passes */
	erase_pass_t  **em_passes; /* Passes definitions */
} erase_method_t;

/*
 * Callbacks types:
 */

typedef void (*erase_cb_file_t)  (const char *);
typedef void (*erase_cb_method_t)(const char *, const erase_method_t *);
typedef void (*erase_cb_pass_t)  (const char *, const erase_method_t *, uint64_t);
typedef void (*erase_cb_block_t) (const char *, const erase_method_t *, uint64_t, uint64_t, uint64_t);
typedef void (*erase_cb_msg_t)   (const char *);
/*
 * Misc macro definitions:
 * =======================
 *
 * ERASE_REFCNT_STATIC:
 */

#define ERASE_REFCNT_STATIC	~0UL

/* initialize.c */
extern erase_status_t erase_initialize(void);
extern int            erase_option_isset_back_name_resolution(void);
extern int            erase_option_isset_debug(void);
extern int            erase_option_isset_simulation(void);
extern int            erase_option_isset_verbose(void);
extern void           erase_option_set_back_name_resolution(int);
extern void           erase_option_set_debug(int);
extern void           erase_option_set_simulation(int);
extern void           erase_option_set_verbose(int);

/* callbacks.c */
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

/* debug.c */
extern void erase_debug(const char *format, ...);
extern void erase_dump_memory(void *, size_t);

/* definitions.c */
extern erase_status_t erase_definition_read_from_fd(int, const char *, size_t);
extern erase_status_t erase_definition_read_from_file(const char *);
extern erase_status_t erase_definition_write_to_fd(int, const char *, size_t, erase_method_t *, int);
extern erase_status_t erase_definition_write_to_file(const char *, erase_method_t *, int);
extern erase_status_t erase_definition_write_all_fd(int, const char *, size_t);
extern erase_status_t erase_definition_write_all_file(const char *);

/* error.c */
extern void        erase_print_error(erase_status_t, const char *format, ...);
extern const char *erase_error_string(erase_status_t);

/* info.c */
extern void erase_info(const char *format, ...);

/* methods.c */
extern erase_status_t erase_method_add_pass     (erase_method_t *, erase_pass_t *);
extern erase_status_t erase_method_create       (const char *, const char *, erase_method_t **);
extern erase_status_t erase_method_destroy      (erase_method_t *);
extern erase_status_t erase_method_find         (const char *, erase_method_t **);
extern erase_status_t erase_method_register     (erase_method_t *);
extern erase_status_t erase_method_unregister   (const char *, erase_method_t **);
extern erase_status_t erase_method_validate     (erase_method_t *);
extern erase_status_t erase_method_validate_db  (void);
extern erase_status_t erase_method_walk         (void *, erase_status_t (*)(void *, erase_method_t *));
extern erase_status_t erase_methods_clear_marker(void);

/* passes.c */
extern erase_status_t erase_pass_create_bit_lrotate(unsigned long, erase_pass_t **);
extern erase_status_t erase_pass_create_bit_rrotate(unsigned long, erase_pass_t **);
extern erase_status_t erase_pass_create_byte(unsigned long, erase_pass_t **);
extern erase_status_t erase_pass_create_byte_lrotate(unsigned long, erase_pass_t **);
extern erase_status_t erase_pass_create_byte_rrotate(unsigned long, erase_pass_t **);
extern erase_status_t erase_pass_create_invert(erase_pass_t **);
extern erase_status_t erase_pass_create_method(erase_method_t *, erase_pass_t **);
extern erase_status_t erase_pass_create_method_name(const char *, erase_pass_t **);
extern erase_status_t erase_pass_create_pattern(unsigned char *, size_t, erase_pass_t **);
extern erase_status_t erase_pass_create_random(int, erase_pass_t **);
extern erase_status_t erase_pass_create_rndbyte(int, erase_pass_t **);
extern erase_status_t erase_pass_destroy(erase_pass_t *);

/* remove.c */
extern erase_status_t erase_remove(const char *, erase_method_t *);

/* error.c */
extern const char *erase_error_string(erase_status_t);

/* warning.c */
extern void erase_warnings_walk(erase_cb_msg_t);

#endif /*! __ERASE_H__*/
