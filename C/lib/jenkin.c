/*-                                               -*- c -*-
 * Copyright (c) 2010
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
 *
 * Description :
 *	Jenkin's hash function for lookup tables
 */

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <CCA/jenkin.h>

extern uint32_t jenkin    (const void *, size_t, uint32_t);
extern uint32_t jenkin0   (const void *, size_t);
extern uint32_t jenkin_str(const char *str);

#define hashsize(n) ((uint32_t)1 << (n))
#define hashmask(n) (hashsize(n) - 1)

/*
  --------------------------------------------------------------------
  mix -- mix 3 32-bit values reversibly.
  For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
  * If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
  * If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
  mix() was built out of 36 single-cycle latency instructions in a
  structure that could supported 2x parallelism, like so:
  a -= b;
  a -= c; x = (c>>13);
  b -= c; a ^= x;
  b -= a; x = (a<<8);
  c -= a; b ^= x;
  c -= b; x = (b>>13);
  ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
  --------------------------------------------------------------------
*/
#define mix(a, b, c) {                          \
                a -= b; a -= c; a ^= (c >> 13); \
                b -= c; b -= a; b ^= (a <<  8); \
                c -= a; c -= b; c ^= (b >> 13); \
                a -= b; a -= c; a ^= (c >> 12); \
                b -= c; b -= a; b ^= (a << 16); \
                c -= a; c -= b; c ^= (b >>  5); \
                a -= b; a -= c; a ^= (c >>  3); \
                b -= c; b -= a; b ^= (a << 10); \
                c -= a; c -= b; c ^= (b >> 15); \
        }
/*
  --------------------------------------------------------------------
  jenkin() -- hash a variable-length key into a 32-bit value
  data    : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
  Returns a 32-bit value.  Every bit of the key affects every bit of
  the return value.  Every 1-bit and 2-bit delta achieves avalanche.
  About 6*len+35 instructions.

  The best hash table sizes are powers of 2.  There is no need to do
  mod a prime (mod is sooo slow!).  If you need less than 32 bits,
  use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
  In which case, the hash table should have hashsize(10) elements.

  If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

  By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
  code any way you wish, private, educational, or commercial.  It's free.

  See http://burtleburtle.net/bob/hash/evahash.html
  Use for hash table lookup, or anything where one collision in 2^^32 is
  acceptable.  Do NOT use for cryptographic purposes.
  --------------------------------------------------------------------
*/

uint32_t jenkin(const void *data, size_t length, uint32_t initval)
{
        register       uint32_t  a;
        register       uint32_t  b;
        register       uint32_t  c;
	register const uint8_t  *k;
        register       uint32_t  len;

        /* Set up the internal state */
	k = (const uint8_t *)data;
        len = length;
        a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
        c = initval;         /* the previous hash value */

        /*---------------------------------------- handle most of the key */
        while (len >= 12)
        {
                a += (((uint32_t)k[ 0] <<  0) + ((uint32_t)k[ 1] <<  8) + ((uint32_t)k[ 2] << 16) + ((uint32_t)k[ 3] <<24));
                b += (((uint32_t)k[ 4] <<  0) + ((uint32_t)k[ 5] <<  8) + ((uint32_t)k[ 6] << 16) + ((uint32_t)k[ 7] <<24));
                c += (((uint32_t)k[ 8] <<  0) + ((uint32_t)k[ 9] <<  8) + ((uint32_t)k[10] << 16) + ((uint32_t)k[11] <<24));
                mix(a,b,c);
                k += 12; len -= 12;
        }

        /*------------------------------------- handle the last 11 bytes */
        c += length;
        switch(len)              /* all the case statements fall through */
        {
        case 11: c += ((uint32_t)k[10] << 24);
        case 10: c += ((uint32_t)k[ 9] << 16);
        case 9 : c += ((uint32_t)k[ 8] <<  8);
                /* the first byte of c is reserved for the length */
        case 8 : b += ((uint32_t)k[ 7] << 24);
        case 7 : b += ((uint32_t)k[ 6] << 16);
        case 6 : b += ((uint32_t)k[ 5] <<  8);
        case 5 : b += ((uint32_t)k[ 4] <<  0);
        case 4 : a += ((uint32_t)k[ 3] << 24);
        case 3 : a += ((uint32_t)k[ 2] << 16);
        case 2 : a += ((uint32_t)k[ 1] <<  8);
        case 1 : a += ((uint32_t)k[ 0] <<  0);
                /* case 0: nothing left to add */
        }
        mix(a,b,c);
       /*-------------------------------------------- report the result */
        return c;
}

uint32_t jenkin0(const void *data, size_t length)
{
	return jenkin(data, length, (uint32_t)0);
}

uint32_t jenkin_str(const char *str)
{
	return jenkin((const void *)str, strlen(str), 0);
}
