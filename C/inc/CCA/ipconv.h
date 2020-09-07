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
 */

#ifndef __CC_IPCONV_H__
#define __CC_IPCONV_H__
struct sockaddr;

#define CC_IPCONV_QUERY		0
#define CC_IPCONV_IPV4ONLY	1
#define CC_IPCONV_IPV6ONLY	2
#define CC_IPCONV_IPV4FIRST	3
#define CC_IPCONV_IPV6FIRST	4

extern struct sockaddr *cc_address2ip (const char *, struct sockaddr *);
extern struct sockaddr *cc_address2ip4(const char *, struct sockaddr *);
extern struct sockaddr *cc_address2ip6(const char *, struct sockaddr *);
extern void             cc_addressfree(struct sockaddr *);
extern int              cc_ipconv_behaviour(int);
#endif /*!__CC_IPCONV_H__*/
