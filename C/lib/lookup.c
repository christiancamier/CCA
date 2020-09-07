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

#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>

#include <CCA/memory.h>
#include <CCA/util.h>

#define __CC_LOOKUP_INTERNAL__
#include <CCA/lookup.h>

/* Local datatypes definition */
struct lookup_entry_st {
	struct lookup_entry_st *le_prev;
	struct lookup_entry_st *le_next;
	uint64_t                le_kval;
	const  void            *le_key;
	const  void            *le_data;
};

struct lookup_table_st {
	cc_lookup_comp_t        lt_comp;
	cc_lookup_hash_t        lt_hash;
	cc_lookup_free_t        lt_free;
	pthread_mutex_t         lt_mutex;
	size_t                  lt_len;
	struct lookup_entry_st *lt_ent[1];
};

struct lookup_freelst_st {
	struct lookup_freelst_st *fl_prev;
	struct lookup_freelst_st *fl_next;
	void                     *fl_spare;
	pthread_mutex_t           fl_mutex;
	size_t                    fl_len;
};

#define LOOKUP_TABLE_HEAD_SZ (size_t)(((struct lookup_table_st *)0)->lt_ent)

/* Global functions */
extern CC_LOOKUP           cc_lookup_create (size_t, cc_lookup_comp_t, cc_lookup_hash_t, cc_lookup_free_t);
extern void                cc_lookup_destroy(CC_LOOKUP);
extern cc_lookup_status_t  cc_lookup_add    (CC_LOOKUP, const void *, void  *);
extern cc_lookup_status_t  cc_lookup_del    (CC_LOOKUP, const void *, void **);
extern cc_lookup_status_t  cc_lookup_search (CC_LOOKUP, const void *, void **);
extern void                cc_lookup_lock   (CC_LOOKUP);
extern void                cc_lookup_unlock (CC_LOOKUP);

/* Local defined functions */
static struct lookup_entry_st *entry_alloc(void);
static struct lookup_entry_st *entry_find(CC_LOOKUP, const void *, uint64_t *, size_t *, struct lookup_entry_st ***);
static void                    entry_free(struct lookup_entry_st *);
static struct lookup_table_st *table_alloc(size_t);
static void                    table_free(struct lookup_table_st *);

/* LOcal data */
static pthread_mutex_t           mut_freelist[1] = { PTHREAD_MUTEX_INITIALIZER };
static struct lookup_entry_st   *ent_freelist = NULL;
static struct lookup_freelst_st *tbl_freelist = NULL;



CC_LOOKUP cc_lookup_create(size_t size, cc_lookup_comp_t fn_comp, cc_lookup_hash_t fn_hash, cc_lookup_free_t fn_free)
{
	CC_LOOKUP new_lktabl = NULL;

	if(NULL != (new_lktabl = table_alloc(size)))
	{
		new_lktabl->lt_comp = fn_comp;
		new_lktabl->lt_hash = fn_hash;
		new_lktabl->lt_free = fn_free;
	}
	return new_lktabl;
}

void cc_lookup_destroy(CC_LOOKUP table)
{
	struct lookup_entry_st **pt;
	struct lookup_entry_st  *pe;
	struct lookup_entry_st  *po;
	struct lookup_entry_st  *ph;
	size_t                   ix;

	cc_lookup_lock(table);
	for(ix = 0, pt = table->lt_ent; ix < table->lt_len; ix += 1)
	{
		if(table->lt_free)
		{
			if((ph = *(pt++)))
			{
				for(pe = ph;pe; po = pe, pe = pe->le_next)
				{
					po = pe;
					table->lt_free(pe->le_data);
				}
				pthread_mutex_lock(mut_freelist);
				po->le_next  = ent_freelist;
				ent_freelist = ph;
				pthread_mutex_unlock(mut_freelist);
			}
		}
		
	}
	table_free(table);
	cc_lookup_unlock(table);
	return;
}

void cc_lookup_lock(CC_LOOKUP table)
{
	pthread_mutex_lock(&(table->lt_mutex));
	return;
}

void cc_lookup_unlock(CC_LOOKUP table)
{
	pthread_mutex_unlock(&(table->lt_mutex));
	return;
}

cc_lookup_status_t cc_lookup_add(CC_LOOKUP table, const void *key, void  *data)
{
	struct lookup_entry_st **head;
	struct lookup_entry_st  *pent;
	uint64_t                 hval;

	if(NULL != (pent = entry_find(table, key, &hval, NULL, &head)))
		return CC_LOOKUP_DUPKEY;
	if(NULL == (pent = entry_alloc()))
		return CC_LOOKUP_SYSERR;
	pent->le_kval =  hval;
	pent->le_key  =  key;
	pent->le_data =  data;
	pent->le_prev = NULL;
	pent->le_next = *head;
	if(*head)
		(*head)->le_prev = pent;
	*head = pent;
	return CC_LOOKUP_OK;
}

cc_lookup_status_t cc_lookup_del(CC_LOOKUP table, const void *key, void **deleted)
{
	struct lookup_entry_st **head;
	struct lookup_entry_st  *pent;
	uint64_t                 hval;
	
	if(NULL == (pent = entry_find(table, key, &hval, NULL, &head)))
		return CC_LOOKUP_NOENT;

	if(pent->le_prev)  pent->le_prev->le_next = pent->le_next;
	else              *head = pent->le_next;
	if(pent->le_next)  pent->le_next->le_prev = pent->le_prev;
	if(*deleted) *deleted = (void *)pent->le_data;
	if(table->lt_free) table->lt_free(pent->le_data);
	entry_free(pent);
	return CC_LOOKUP_OK;
}

cc_lookup_status_t cc_lookup_search(CC_LOOKUP table, const void *key, void **returned)
{
	struct lookup_entry_st  *pent;

	if(NULL == (pent = entry_find(table, key, NULL, NULL, NULL)))
		return CC_LOOKUP_NOENT;
	if(*returned) *returned = (void *)pent->le_data;
	return CC_LOOKUP_OK;
}

static struct lookup_entry_st *entry_alloc(void)
{
	struct lookup_entry_st *newent;

	pthread_mutex_lock(mut_freelist);
	if(NULL == ent_freelist)
	{
		newent          = ent_freelist;
		ent_freelist    = newent->le_next;
		newent->le_next = NULL;
	}
	else
	{
		newent = CC_TALLOC(struct lookup_entry_st, 1);
	}
	pthread_mutex_unlock(mut_freelist);

	return newent;
}

static struct lookup_entry_st *entry_find(CC_LOOKUP table, const void *key, uint64_t *rhash, size_t *rhent, struct lookup_entry_st ***rpent)
{
	struct lookup_entry_st *eptr;
	uint64_t                hval = table->lt_hash(key);
	size_t                  hent = hval % table->lt_len;

	if(rhash) *rhash = hval;
	if(rhent) *rhent = hent;
	if(rpent) *rpent = table->lt_ent + hent;

	for(eptr = table->lt_ent[hent]; eptr; eptr = eptr->le_next)
	{
		if(hval == eptr->le_kval && 0 == table->lt_comp(eptr->le_key, key))
		{
			return eptr;
		}
	}
	return NULL;
}

static void entry_free(struct lookup_entry_st *entry)
{
	pthread_mutex_lock(mut_freelist);
	entry->le_next = ent_freelist;
	entry->le_prev = NULL;
	entry->le_kval = 0;
	entry->le_key  = NULL;
	entry->le_data = NULL;
	ent_freelist   = entry;
	pthread_mutex_unlock(mut_freelist);
	return;
}

static struct lookup_table_st *table_alloc(size_t len)
{
	struct lookup_table_st   *newret;
	struct lookup_freelst_st *newlst;

	pthread_mutex_lock(mut_freelist);
	for(newlst = tbl_freelist; newlst; newlst = newlst->fl_next)
	{
		if(newlst->fl_len == len)
		{
			if(newlst->fl_prev) newlst->fl_prev->fl_next = newlst->fl_next;
			else                tbl_freelist = newlst->fl_next;
			if(newlst->fl_next) newlst->fl_next->fl_prev = newlst->fl_prev;
			newlst = newlst;
			break;
		}
	}
	pthread_mutex_unlock(mut_freelist);
	if(NULL != newlst)
	{
		newret = (struct lookup_table_st *)newlst;
		newret->lt_comp = NULL;
		newret->lt_hash = NULL;
	}
	else
	{
		if(NULL != (newret = (struct lookup_table_st *)cc_malloc(LOOKUP_TABLE_HEAD_SZ + len * sizeof(struct lookup_entry_st *))))
		{
			newret->lt_len = len;
			pthread_mutex_init(&(newret->lt_mutex), NULL);
		}
	}
	return newret;
}

static void table_free(struct lookup_table_st *table)
{
	struct lookup_freelst_st  *fltbl = (struct lookup_freelst_st *)table;
	size_t                     index;
	struct lookup_entry_st   **entry;

	table->lt_comp = NULL;
	table->lt_hash = NULL;
	table->lt_free = NULL;
	for(entry = table->lt_ent, index = 0; index < table->lt_len; *(entry++) = NULL, index += 1);
	fltbl->fl_prev = NULL;
	pthread_mutex_lock(mut_freelist);
	fltbl->fl_next = tbl_freelist;
	tbl_freelist   = fltbl;
	pthread_mutex_unlock(mut_freelist);
	return;
}
