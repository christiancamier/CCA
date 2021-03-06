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

#ifndef __CC_LOOKUP_H__
#define __CC_LOOKUP_H__

#ifdef __CC_LOOKUP_INTERNAL__
struct lookup_table_st;
typedef struct lookup_table_st *CC_LOOKUP;
#else
typedef void *CC_LOOKUP;
#endif

typedef enum cc_lookup_status_en {
	CC_LOOKUP_SYSERR	= -1, /* System error    */
	CC_LOOKUP_OK		= 0,  /* No error        */
	CC_LOOKUP_NOENT,	      /* Entry not found */
	CC_LOOKUP_DUPKEY	      /* Duplicate key   */
} cc_lookup_status_t;
	

typedef int      (*cc_lookup_comp_t)(const void *, const void *);
typedef uint64_t (*cc_lookup_hash_t)(const void *);
typedef void     (*cc_lookup_free_t)(const void *);

/*
 * cc_lookup_create:
 *	Create a lookup table
 *
 * Synopsis:
 *	CC_LOOKUP cc_lookup_create(size_t size, cc_lookup_cmp_t comparator, cc_lookup_hash_t hasher);
 *
 * Arguments:
 *  . size:
 *      Size of the hash table (number of entries).
 *	For the size of the tables, the use of an odd first number is strongly advised for reasons of distribution.
 *	First possiblities :
 *	3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109,
 *	113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
 *	239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367,
 *	373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
 *	503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643,
 *	647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797,
 *	809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947,
 *	953, 967, 971, 977, 983, 991, 997, etc.
 *
 *  . comparator:
 *      Comparator function.
 *	If comparator is NULL, defaul function will be "strcmp".
 *  . hasher:
 *      Hash function.
 *	If hasher is NULL, default function will be "jenkin".
 *
 * Returns:
 *	An lookup header or NULL if an error occurs.
 */

extern CC_LOOKUP cc_lookup_create(size_t, cc_lookup_comp_t, cc_lookup_hash_t, cc_lookup_free_t);
extern void      cc_lookup_destroy(CC_LOOKUP);

/*
 * cc_lookup_add:
 *	Add an element to lookup
 *
 * Synopsis:
 *	int cc_lookup_add(CC_HASTABLE lookup, CC_LOOKUP_HDR element);
 *
 * Arguments:
 *   . lookup : lookup to use.
 *   . element   : Element to add. The header key field will be used.
 *
 * Returns:
 *	. CC_LOOKUP_ERR_NOERROR : Element has been successfuly added.
 *	. CC_LOOKUP_ERR_EXISTS  : Duplicate key.
 */

extern cc_lookup_status_t cc_lookup_add   (CC_LOOKUP, const void *, void  *);
extern cc_lookup_status_t cc_lookup_del   (CC_LOOKUP, const void *, void **);
extern cc_lookup_status_t cc_lookup_search(CC_LOOKUP, const void *, void **);

#endif /* ! __CC_LOOKUP_H__ */
