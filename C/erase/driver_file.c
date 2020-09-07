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
#include <stdlib.h>
#include <unistd.h>

#include <CCA/util.h>

#include "erase_internal.h"

static erase_status_t file_is_a(erase_file_t *);
static erase_status_t file_initialize(erase_file_t *);
static erase_status_t file_process(erase_file_t *, erase_method_t *, uint64_t);
static erase_status_t file_finalize(erase_file_t *);

erase_driver_t file_driver = {
	.name         = "Block device driver",
	.is_a         = file_is_a,
	.initialize   = file_initialize,
	.process_pass = file_process,
	.finalize     = file_finalize
};

static erase_status_t file_is_a(erase_file_t *file)
{
	struct stat     st;
	erase_buffer_t *buffer;
	erase_status_t  status;

	buffer = CC_TNULL(erase_buffer_t);

	if(-1 == stat(file->fname, &st))
		return ERA_ST_SYSTEM_ERROR;

	if(!S_ISREG(st.st_mode))
		return ERA_ST_DONT_MATCH;

	if(!is_simulation() && ERA_ST_OK != (status = _erase_buffer_create((size_t)st.st_blksize, &buffer)))
		return status;

	file->fsize  = (uint64_t)st.st_size;
	file->blkcur = 0ULL;
	file->blkcnt = (uint64_t)((st.st_size + st.st_blksize - 1) / st.st_blksize);
	file->driver = &file_driver;
	file->buffer = buffer;
	return ERA_ST_OK;
}

static erase_status_t file_initialize(erase_file_t *file)
{
	(void)file;
	return ERA_ST_OK;
}

static erase_status_t file_process(erase_file_t *file, erase_method_t *method, uint64_t passno)
{
	erase_pass_t   *pass;
	erase_buffer_t *buffer;
	uint64_t        nblocks;
	uint64_t        cblock;

	buffer  = file->buffer;
	pass    = *(method->em_passes + passno);
	nblocks = file->blkcnt;
	(void)lseek(file->fdesc, (off_t)0, SEEK_SET);
	_erase_fill_buffer_reset(buffer);
	for(cblock = 0; cblock < nblocks; cblock += 1)
	{
		_erase_cb_block_start(file->fname, method, passno, cblock, nblocks);
		if(!is_simulation())
		{
			_erase_fill_buffer(buffer, pass);
			if(buffer->bu_size != write(file->fdesc, (void *)(buffer->bu_cdata), buffer->bu_size))
				return ERA_ST_SYSTEM_ERROR;
		}
		_erase_cb_block_end(file->fname, method, passno, cblock, nblocks);
	}
	return ERA_ST_OK;
}

static erase_status_t file_finalize(erase_file_t *file)
{
	(void)file;
	return ERA_ST_OK;
}

static __attribute__((constructor)) void drv_file_ctor(void)
{
	_erase_driver_register(&file_driver);
}
