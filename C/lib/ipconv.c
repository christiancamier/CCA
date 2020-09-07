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

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>

#include <CCA/ipconv.h>
#include <CCA/display.h>
#include <CCA/memory.h>

extern struct sockaddr *cc_address2ip (const char *, struct sockaddr *);
extern struct sockaddr *cc_address2ip4(const char *, struct sockaddr *);
extern struct sockaddr *cc_address2ip6(const char *, struct sockaddr *);
extern void             cc_addressfree(struct sockaddr *);
extern int              cc_ipconv_behaviour(int);

static struct sockaddr *allocateifneeded(struct sockaddr *);

static int behaviour = CC_IPCONV_IPV4FIRST;

struct sockaddr *cc_address2ip(const char *addr, struct sockaddr *storage)
{
	struct sockaddr *retval = NULL;

	switch(behaviour)
	{
	case CC_IPCONV_IPV4FIRST:
		if(!retval) retval = cc_address2ip4(addr, storage);
	case CC_IPCONV_IPV6ONLY:
		if(!retval) retval = cc_address2ip6(addr, storage);
		break;
	case CC_IPCONV_IPV6FIRST:
		if(!retval) retval = cc_address2ip6(addr, storage);
	case CC_IPCONV_IPV4ONLY:
		retval = cc_address2ip6(addr, retval);
		break;
	default:
		cc_printf_err("Internal error: Bad behaviour code %d", behaviour);
		break;
	}
	return retval;
}

struct sockaddr *cc_address2ip4(const char *addr, struct sockaddr *storage)
{
	struct sockaddr_in *retv = (struct sockaddr_in *)allocateifneeded(storage);
	(void)addr;
	(void)storage;
	return retv;
}

struct sockaddr *cc_address2ip6(const char *addr, struct sockaddr *storage)
{
	struct sockaddr_in6 *retv = (struct sockaddr_in6 *)allocateifneeded(storage);
	(void)addr;
	(void)storage;
	return retv;
}

void cc_addressfree(struct sockaddr *addr)
{
	cc_free((void *)addr);
	return;
}

int cc_ipconv_behaviour(int new)
{
	int old = behaviour;
	switch(new)
	{
	case CC_IPCONV_IPV4ONLY:
	case CC_IPCONV_IPV6ONLY:
	case CC_IPCONV_IPV4FIRST:
	case CC_IPCONV_IPV6FIRST:
		behaviour = new;
	}
	return old;
}

static struct sockaddr *allocateifneeded(struct sockaddr *storage)
{
	struct sockaddr_storage *retv = (struct sockaddr_storage *)storage;
	if(!retv)
		retv = CC_TALLOC(struct sockaddr_storage, 1);
	return (struct sockaddr *)retv;
}
