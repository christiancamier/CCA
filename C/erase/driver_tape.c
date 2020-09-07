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
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "erase_internal.h"

static erase_status_t   tape_is_a(erase_file_t *);
static erase_status_t tape_initialize(erase_file_t *);
static erase_status_t tape_process(erase_file_t *, erase_method_t *, uint64_t);
static erase_status_t tape_finalize(erase_file_t *);

erase_driver_t tape_driver = {
	.name         = "Tape device driver",
	.is_a         = tape_is_a,
	.initialize   = tape_initialize,
	.process_pass = tape_process,
	.finalize     = tape_finalize
};

static int tape_operation(int fdesc, int operation, int count)
{
	struct mtop mtop;

	mtop.mt_op    = ( short )operation;
	mtop.mt_count = (daddr_t)count;
	return ioctl(fdesc, operation, &mtop);
}

#define tape_nop(f)	tape_operation(f, MTNOP,   1)
#define tape_erase(f)	tape_operation(f, MTERASE, 1)
#define tape_offl(f)	tape_operation(f, MTOFFL,  1)
#define tape_reset(f)	tape_operation(f, MTRESET, 1)
#if   defined(MTRETEN)
# define tape_reten(f)	tape_operation(f, MTRETEN, 1)
#elif defined(MTRETENS)
# define tape_reten(f)	tape_operation(f, MTRETENS, 1)
#else
# define tape_reten(f)
#endif
#define tape_rew(f)	tape_operation(f, MTREW,   1)
#define tape_weof(f)	tape_operation(f, MTWEOF,  1)

static erase_status_t tape_is_a(erase_file_t *file)
{
	struct stat     st;
	erase_buffer_t *buffer;
	erase_status_t  status;

	buffer = CC_TNULL(erase_buffer_t);

	if(-1 == stat(file->fname, &st))
		return ERA_ST_SYSTEM_ERROR;

	if(!S_ISCHR(st.st_mode))
		return ERA_ST_DONT_MATCH;

	if(-1 == tape_nop(file->fdesc))
		return ERA_ST_DONT_MATCH;

	if(!is_simulation() && ERA_ST_OK != (status = _erase_buffer_create((size_t)st.st_blksize, &buffer)))
		return status;

	file->fsize  = 0ULL;
	file->blkcur = 0ULL;
	file->blkcnt = is_simulation() ? 4096ULL : 0ULL;
	file->driver = &tape_driver;
	file->buffer = buffer;
	
	return ERA_ST_OK;
}

static erase_status_t tape_initialize(erase_file_t *file)
{
	(void)tape_reten(file->fdesc);
	(void)tape_rew(file->fdesc);
	return ERA_ST_OK;
}

static erase_status_t tape_process(erase_file_t *file, erase_method_t *method, uint64_t passno)
{
	erase_pass_t   *pass;
	erase_buffer_t *buffer;
	uint64_t        cblock;
	uint64_t        nblocks;

	buffer  = file->buffer;
	pass    = *(method->em_passes + passno);

	_erase_fill_buffer_reset(buffer);
	if(is_simulation())
	{
		nblocks = file->blkcnt;
		for(cblock = 0; cblock < nblocks; cblock += 1)
		{
			_erase_cb_block_start(file->fname, method, passno, cblock, nblocks);
			_erase_fill_buffer(buffer, pass);
			_erase_cb_block_end(file->fname, method, passno, cblock, nblocks);
		}
	}
	else
	{
		(void)tape_rew(file->fdesc);
		errno  = 0;
		cblock = 0;
		while(errno != ENOSPC)
		{
			_erase_cb_block_start(file->fname, method, passno, cblock, 0);
			_erase_fill_buffer(buffer, pass);
			if(write(file->fdesc, (void *)buffer->bu_cdata, buffer->bu_size) != buffer->bu_size && errno != ENOSPC)
				return ERA_ST_SYSTEM_ERROR;
			_erase_cb_block_end(file->fname, method, passno, cblock, 0);
			cblock += 1;
		}
		(void)tape_rew(file->fdesc);
	}
	return ERA_ST_OK;
}

static erase_status_t tape_finalize(erase_file_t *file)
{
	(void)tape_erase(file->fdesc);
	(void)tape_reten(file->fdesc);
	(void)tape_weof(file->fdesc);
	(void)tape_offl(file->fdesc);
	return ERA_ST_OK;
}

static __attribute__((constructor)) void drv_tape_ctor(void)
{
	_erase_driver_register(&tape_driver);
}
