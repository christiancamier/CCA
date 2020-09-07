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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CCA/memory.h>

#include "erase_internal.h"

struct linked_method_st;

extern erase_status_t _erase_method_process(erase_file_t *, erase_method_t *);

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

static erase_status_t linked_insert      (struct linked_method_st *);
static erase_status_t linked_find        (const char *, struct linked_method_st **);
static erase_status_t linked_remove      (struct linked_method_st *);
static erase_status_t resolve_pass_method(erase_pass_t *);
static erase_status_t valid_sub_methods  (erase_method_t *);

struct linked_method_st {
	struct linked_method_st *prev;
	struct linked_method_st *next;
	erase_method_t          *method;
};

static struct linked_method_st *first = (struct linked_method_st *)NULL;
static struct linked_method_st *last  = (struct linked_method_st *)NULL;

static erase_status_t linked_find(const char *name, struct linked_method_st **linked)
{
	struct linked_method_st *current;

	erase_debug("linked_find(%s, %p)", name, linked);
	for(current = first; current; current = current->next)
	{
		if(0 == strcmp(current->method->em_name, name))
		{
			if(linked)
				*linked = current;
			return ERA_ST_OK;
		}
	}
	return ERA_ST_UNKNOWN_METHOD;
}

static erase_status_t linked_insert(struct linked_method_st *element)
{
	erase_debug("linked_insert(%p) // first = %p, last = %p", element, first, last);
	element->prev = last;
	element->next = NULL;
	if(!first)
	{
		first = last = element;
	}
	else
	{
		last->next = element;
		last       = element;
	}
	return ERA_ST_OK;
}

static erase_status_t linked_remove(struct linked_method_st *element)
{
	erase_debug("linked_remove(%p) // first = %p", element, first);
	if(CC_TNULL(struct linked_method_st) != element->prev)
		element->prev->next = element->next;
	else
		first = element->next;
	if(CC_TNULL(struct linked_method_st) != element->next)
		element->next->prev = element->prev;
	else
		last = element->prev;
	_erase_memory_free(element);
	return ERA_ST_OK;
}

static erase_status_t resolve_pass_method(erase_pass_t *pass)
{
	erase_status_t  status;
	erase_method_t *method;

	erase_debug("resolve_pass_method(%p)", pass);
	status  = ERA_ST_OK;

	if(0 != ERASE_MODE_ARG(pass->ep_mode))
		return status;

	if(ERA_ST_OK == (status = erase_method_find(pass->ep_method_name, &method)))
	{
		(void)_erase_memory_free(pass->ep_method_name);
		pass->ep_mode |= 0xFF;
		pass->ep_method_def = method;
		method->em_refcnt += 1;
	}
	return status;
}

static erase_status_t valid_sub_methods(erase_method_t *method)
{
	erase_status_t   status;
	erase_pass_t   **cpass;
	size_t           npass;

	erase_debug("valid_sub_methods(%p)", method);
	if(method->em_marker)
	{
		fprintf(stderr, "Method %s: Cycle dectected\n", method->em_name);
		return ERA_ST_CYCLE_DETECTED;
	}
	method->em_marker = 1;
	status            = ERA_ST_OK;
	for(npass = method->em_npass, cpass = method->em_passes; npass > 0; npass -= 1, cpass += 1)
	{
		if(ERASE_MODE_METHOD_CODE != ERASE_MODE_CODE((*cpass)->ep_mode))
			continue;
		if(ERA_ST_OK != (status = resolve_pass_method(*cpass)))
			break;
		status = valid_sub_methods((*cpass)->ep_method_def);
		if(ERA_ST_CYCLE_DETECTED == status)
			fprintf(stderr, "From %s\n", method->em_name);
		if(ERA_ST_OK != status)
			break;
	}
	method->em_marker = 0;
	return status;
}

erase_status_t erase_method_register(erase_method_t *method)
{
	struct linked_method_st *linked;
	erase_status_t           status;

	erase_debug("erase_method_register(%s)", method->em_name);
	if(ERA_ST_OK == linked_find(method->em_name, NULL))
		return ERA_ST_METHOD_EXISTS;
	if(ERA_ST_OK == (status = _erase_memory_alloc(sizeof(struct linked_method_st), (void **)&linked)))
	{
		linked->prev   = CC_TNULL(struct linked_method_st);
		linked->next   = CC_TNULL(struct linked_method_st);
		linked->method = method;
		status = linked_insert(linked);
	}
	//erase_dump_memory(method, sizeof(erase_method_t));
	return status;
}

erase_status_t erase_method_unregister(const char *name, erase_method_t **method)
{
	struct linked_method_st *linked;
	erase_status_t           status;

	if(ERA_ST_OK == (status = linked_find(name, &linked)))
	{
		if(CC_TNULL(erase_method_t *) != method)
			*method = linked->method;
		status = linked_remove(linked);
	}
	return status;
}

erase_status_t erase_method_find(const char *name, erase_method_t **method)
{
	struct linked_method_st *linked;
	erase_status_t           status;

	if(ERA_ST_OK == (status = linked_find(name, &linked)) && CC_TNULL(erase_method_t *) != method)
		*method = linked->method;
	return status;
}

erase_status_t erase_method_validate(erase_method_t *method)
{
	erase_pass_t **passpt;
	size_t         passno;
	int            has_submethods;
	has_submethods = 0;

	method->em_marker = 0;

	for(passpt = method->em_passes, passno = 0; passno < method->em_npass; passno += 1, passpt += 1)
	{
		erase_pass_t *pass = *passpt;

		if(ERASE_MODE_METHOD_CODE == ERASE_MODE_CODE(pass->ep_mode))
		{
			has_submethods = 1;
			if(0 == ERASE_MODE_ARG(pass->ep_mode))
			{
				erase_status_t  s;
				s = resolve_pass_method(pass);
				if(ERA_ST_UNKNOWN_METHOD == s)
				{
					fprintf(stderr, "Cannot find method `%s'\n", method->em_name);
					fprintf(stderr, "Method `%s' is incomplete\n", pass->ep_method_name);
					return ERA_ST_METHOD_INCOMPLETE;
				}
				if(ERA_ST_OK != s)
					return s;
			}
		}
	}
	return has_submethods ? valid_sub_methods(method) : ERA_ST_OK;
}

static erase_status_t _validate_method(void *udata, erase_method_t *method)
{
	(void)udata;
	return erase_method_validate(method);
}

erase_status_t erase_method_validate_db(void)
{
	struct linked_method_st *p;
	for(p = first; p; p = p->next)
		p->method->em_marker = 0;
	return erase_method_walk(NULL, _validate_method);
}

erase_status_t erase_method_walk(void *userdata, erase_status_t (*callback)(void *, erase_method_t *))
{
	struct linked_method_st *C;
	erase_status_t           R;
	erase_debug("Walking all defined methods");
	for(C = first; CC_TNULL(struct linked_method_st) != C; C = C->next)
	{
		if(ERA_ST_OK != (R = callback(userdata, C->method)))
			return R;
	}
	return ERA_ST_OK;
}

erase_status_t erase_method_create(const char *name, const char *description, erase_method_t **method)
{
	char            *d;
	erase_method_t  *m;
	char            *n;
	erase_pass_t   **p;
	erase_status_t   s;

	erase_debug("erase_method_create(%s, %s, %p)", name, description, method);
	d = CC_TNULL(char);
	m = CC_TNULL(erase_method_t);
	n = CC_TNULL(char);
	p = CC_TNULL(erase_pass_t *);

	if(ERA_ST_OK != (s = _erase_strdup(name,        &n))) goto error;
	if(ERA_ST_OK != (s = _erase_strdup(description, &d))) goto error;
	if(ERA_ST_OK != (s = _erase_memory_alloc(sizeof(erase_method_t), (void **)&m)))
		goto error;
	if(ERA_ST_OK != (s = _erase_memory_alloc(ERASE_PASS_INCREMENT * sizeof(erase_pass_t), (void **)&p)))
		goto error;

	m->em_marker = 0;
	m->em_refcnt = 1;
	m->em_name   = n;
	m->em_desc   = d;
	m->em_nslots = ERASE_PASS_INCREMENT;
	m->em_npass  = 0;
	m->em_passes = p;
	*method = m;
	return s;
error:
	CC_PROTECT_ERRNO(if(p) (void)_erase_memory_free(p);
			 if(m) (void)_erase_memory_free(m);
			 if(d) (void)_erase_memory_free(d);
			 if(n) (void)_erase_memory_free(n));
	return s;
}

erase_status_t erase_method_add_pass(erase_method_t *method, erase_pass_t *pass)
{
	erase_status_t status;
	size_t         nnslots;
	erase_pass_t **slots;

	erase_debug("erase_method_add_pass(%p, %p)", method, pass);
	nnslots = method->em_nslots + ERASE_PASS_INCREMENT;
	slots   = method->em_passes;
	if(method->em_nslots == method->em_npass)
	{
		if(ERA_ST_OK != (status = _erase_memory_realloc(method->em_passes, BUFFER_BYTES(nnslots), (void **)&slots)))
			return status;
		method->em_passes = slots;
		method->em_nslots = nnslots;
	}
	*(method->em_passes + method->em_npass) = pass;
	method->em_npass += 1;

	return ERA_ST_OK;
}

erase_status_t erase_method_destroy(erase_method_t *method)
{
	size_t i;

	erase_debug("erase_method_destroy(%p)", method);
	if(ERASE_REFCNT_STATIC == method->em_refcnt)
	{
		erase_debug(" . Method %p is static", method);
		return ERA_ST_IS_STATIC;
	}
	if(method->em_refcnt)
		method->em_refcnt -= 1;
	if(0 < method->em_refcnt)
	{
		erase_debug(" . Reference counter ajusted to %lu", method->em_refcnt);
		return ERA_ST_OK;
	}
	for(i = method->em_npass; i > 0;)
		(void)erase_pass_destroy(method->em_passes[--i]);
	(void)_erase_memory_free(method->em_passes);
	(void)_erase_memory_free(method->em_desc);
	(void)_erase_memory_free(method->em_name);
	(void)_erase_memory_free(method);
	erase_debug(" . method destroyed");
	return ERA_ST_OK;
}

erase_status_t erase_methods_clear_marker(void)
{
	struct linked_method_st *C;
	erase_debug("erase_methods_clear_marker()");
	for(C = first; CC_TNULL(struct linked_method_st) != C; C->method->em_marker = 0, C = C->next);
	return ERA_ST_OK;
}

erase_status_t _erase_method_process(erase_file_t *file, erase_method_t *method)
{
	size_t passno;
	_erase_cb_method_start(file->fname, method);
	for(passno = 0; passno < method->em_npass; passno += 1)
		_erase_pass_process(file, method, (uint64_t)passno);
	_erase_cb_method_end(file->fname, method);
	return ERA_ST_OK;
}
