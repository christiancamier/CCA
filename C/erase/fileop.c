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
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "erase_internal.h"

#ifndef O_SYNC
# define O_SYNC 0
#endif

#ifndef O_DIRECT
# define O_DIRECT 0
#endif

extern erase_status_t  _erase_file_close(erase_file_t *);
extern erase_status_t  _erase_file_open(const char *, erase_file_t **);
extern const char     *_erase_file_type(erase_file_t *);

const char *_erase_file_type(erase_file_t *file)
{
	struct stat st;
	if(-1 != fstat(file->fdesc, &st))
	{
		switch(st.st_mode & S_IFMT)
		{
#ifdef S_IFSOCK
		case S_IFSOCK:	return "socket";
#endif
#ifdef S_IFLNK
		case S_IFLNK:	return "symbolic link";
#endif
#ifdef S_IFREG
		case S_IFREG:	return "regular file";
#endif
#ifdef S_IFBLK
		case S_IFBLK:	return "block device";
#endif
#ifdef S_IFDIR
		case S_IFDIR:	return "directory";
#endif
#ifdef S_IFCHR
		case S_IFCHR:	return "character device";
#endif
#ifdef S_IFIFO
		case S_IFIFO:	return "named pipe";
#endif
		}
	}
	return "unknown";
}

erase_status_t _erase_file_close(erase_file_t *file)
{
	erase_debug("_erase_file_close(%p)", file);
	if(!is_simulation())
		close(file->fdesc);
	if(file->buffer)
		(void)_erase_buffer_destroy(file->buffer);
	if(file->fname)
		(void)_erase_memory_free(file->fname);
	return _erase_memory_free((void *)file);
}

erase_status_t _erase_file_open(const char *filename, erase_file_t **file)
{
	char           *fname;
	erase_file_t   *rfile;
	int             fdesc;
	erase_status_t  status;

	erase_debug("_erase_file_open(%s, %p)", filename, file);
	fdesc = -1;
	if(!is_simulation() && -1 == (fdesc = open(filename, O_WRONLY | O_SYNC | O_DIRECT)))
		return ERA_ST_SYSTEM_ERROR;
	if(ERA_ST_OK != (status = _erase_memory_alloc(sizeof(erase_file_t), (void **)&rfile)))
	{
		CC_PROTECT_ERRNO(if(fdesc != -1) close(fdesc));
		return status;
	}
	if(ERA_ST_OK != (status = _erase_strdup(filename, &fname)))
	{
		CC_PROTECT_ERRNO(
			if(fdesc != -1)
				close(fdesc);
			_erase_memory_free(rfile));
		return status;
	}

	rfile->fname  = fname;
	rfile->fdesc  = fdesc;
	rfile->fsize  = 0ULL;
	rfile->blkcur = 0ULL;
	rfile->blkcnt = 0ULL;
	rfile->driver = CC_TNULL(erase_driver_t);
	rfile->buffer = CC_TNULL(erase_buffer_t);

	if(ERA_ST_OK != (status = _erase_driver_find(rfile)))
	{
		CC_PROTECT_ERRNO(
			if(-1 != fdesc)
				close(fdesc);
			(void)_erase_memory_free(fname);
			(void)_erase_memory_free(rfile));
		return status;
	}
	*file = rfile;
	return ERA_ST_OK;
}
