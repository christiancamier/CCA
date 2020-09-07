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

#include <stdint.h>
#include <string.h>

#include <CCA/hashtable.h>
#include <CCA/jenkin.h>
#include <CCA/memory.h>

/* Global functions */
extern CC_HASHTABLE cc_hashtable_create(size_t, cc_hashtable_comp_t, cc_hashtable_hash_t);
extern int          cc_hashtable_add   (CC_HASHTABLE, CC_HASHTABLE_HDR);
extern int          cc_hashtable_del   (CC_HASHTABLE, void *, CC_HASHTABLE_HDR *);
extern int          cc_hashtable_search(CC_HASHTABLE, void *, CC_HASHTABLE_HDR *);

/* Local functions */
static int search(CC_HASHTABLE, const void *, CC_HASHTABLE_HDR **, CC_HASHTABLE_HDR *);

CC_HASHTABLE cc_hashtable_create(size_t size, cc_hashtable_comp_t comp, cc_hashtable_hash_t hash)
{
	size_t       rsz = CC_HASHTABLE_HDR_SZ + size * sizeof(CC_HASHTABLE_HDR);
	CC_HASHTABLE ret;

	if(NULL != (ret = (CC_HASHTABLE)cc_malloc(rsz)))
	{
		ret->fn_comp = comp ? comp : (cc_hashtable_comp_t)strcmp;
		ret->fn_hash = hash ? hash : (cc_hashtable_hash_t)jenkin_str;
	}
	return ret;
}

int cc_hashtable_add(CC_HASHTABLE T, CC_HASHTABLE_HDR H)
{
	uint32_t          hshval = T->fn_hash(H->key);
	CC_HASHTABLE_HDR *header = T->ht_heads + (hshval % T->ht_size);
	CC_HASHTABLE_HDR  p      = *header;

	if(CC_HASHTABLE_ERR_NOERROR == search(T, H->key, NULL, NULL))
		return CC_HASHTABLE_ERR_EXISTS;

	H->cc_ht_val = hshval;
	H->prev      = NULL;
	H->next      = p;
	if(p)
		p->prev = H;
	*header      = H;
	return CC_HASHTABLE_ERR_NOERROR;
}

int cc_hashtable_del(CC_HASHTABLE T, void *K, CC_HASHTABLE_HDR *H)
{
	int                r;
	CC_HASHTABLE_HDR  *e;
	CC_HASHTABLE_HDR   p;
	if(CC_HASHTABLE_ERR_NOERROR != (r = search(T, K, &e, &p)))
		return r;
	if(p->next)
		p->next->prev = p->prev;
	if(p->prev)
		p->prev->next = p->next;
	else
		*e = p->next;
	*H = p;
	return r;
}

int cc_hashtable_search(CC_HASHTABLE T, void *K, CC_HASHTABLE_HDR *H)
{
	return search(T, K, NULL, H);
}


static int search(CC_HASHTABLE T, const void *K, CC_HASHTABLE_HDR **H, CC_HASHTABLE_HDR *E)
{
	uint32_t          hshval = T->fn_hash(K);
	CC_HASHTABLE_HDR *header = T->ht_heads + (hshval % T->ht_size);
	CC_HASHTABLE_HDR  p;
	if(*header)
	{
		for(p = *header; p; p = p->next)
		{
			if(p->cc_ht_val == hshval && T->fn_comp(K, p->key))
			{
				if(H) *H = header;
				if(E) *E = p;
				return CC_HASHTABLE_ERR_NOERROR;
			}
		}
	}
	return CC_HASHTABLE_ERR_NOTFOUND;
}

