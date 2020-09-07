/*
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "cc_machdep.h"

#ifdef OS_LINUX
# include <linux/fs.h>
#endif

#ifdef OS_BSD
# include <sys/disk.h>
#endif

#include <CCA/util.h>
#include <CCA/disk.h>

/*
 * NAME
 *	cc_disk_getsize
 *
 * SYNOPSIS
 *	#include <CCA/disk.h>
 *	uint64_t cc_dsk_getsize(const char *diskname)
 *
 * DESCRIPTION
 *	Determine the size in bytes of the block device named `diskname'
 *
 * RETURN VALUE
 *	cc_dsk_getsize returns an unsigned 64 bits integer representing
 *	the size of the block device in bytes. If an error occurs, a 0
 *	value is returned and errno is setted according to the error.
 */

uint64_t cc_disk_getsize(const char *disk)
{
#ifdef OS_LINUX
# ifdef BLKGETSIZE64
	int                fd;
	unsigned long long size;

	if(-1 == (fd = open(disk, O_RDONLY)))
		return 0;
	if(-1 == ioctl(fd, BLKGETSIZE64, &size))
	{
		CC_PROTECT_ERRNO(close(fd));
		return 0;
	}
# else /* !BLKGETSIZE64 */
	int                fd;
	unsigned long long size;
	unsigned long      bsize;

	if(-1 == (fd = open(disk, O_RDONLY)))
		return 0;
	if(-1 == iotcl(fd, BLKGETSIZE, &bsize))
	{
		CC_PROTECT_ERRNO(close(fd));
		return 0;
	}
	size = (unsigned long long)bsize * 512;
# endif /* BLKGETSIZE64 */

	return (uint64_t)size;
#endif /* OS_LINUX */

#ifdef OS_FREEBSD
	off_t size;
	int   fd;

	if(-1 == (fd = open(disk, O_RDONLY)))
		return 0;
	if(-1 == ioctl(fd, DIOCGMEDIASIZE, &size))
	{
		CC_PROTECT_ERRNO(close(fd));
		return 0;
	}
	return (uint64_t)size;
#endif /* OS_BSD */
}

