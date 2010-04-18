/*-
 * Copyright (c) 2002-2004 Andrey Smirnov.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 *  Checkpoint restore
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "benchmark.h"
#include "debug.h"
#include "files.h"
#include "elf_.h"
#include "image.h"
#include "setup.h"
#include "system.h"
#include "vital.h"

#ifdef WITH_MPICH
#include "mpich/mpich.h"
#endif


static int ckptRestoreHeader(ckptImage * image, size_t size, void **);
static int ckptRestoreData(ckptImage * image, size_t size, void *start);
static int ckptRestoreStack(ckptImage * image, size_t size, void *stack_start);

/**
 * ¬осстановить выполнение программы из контрольной точки
 *
 * \return 0, если была ошибка; при успешном выполнении управление не возвращаетс€
 */

int ckptRestore()
{
	char           *filename;
	ckptImage       image;

	int             type;
	size_t          size;
	void           *start;

	void           *stack_start;
	
	int 		data_read=0;

	filename = ckptGetFinalFilename();

	ckptLog(CKPT_LOG_WARNING, ">>> Restoring from image %s", filename);
	CKPT_BENCHMARK(CKPT_BNCH_REST_BEGIN);

#ifdef WITH_MPICH
	ckptMpichBeforeRestore();
#endif
	ckptPurgeMmaped();
	
	if (!ckptOpenImage(&image, filename))
		return 0;

	while (ckptReadChunk(&image, &type, &size, &start))
	{
		switch (type)
		{
		case CHNK_HEADER:
			ckptRestoreHeader(&image, size, &stack_start);
			break;
		case CHNK_INTERNAL:
		case CHNK_MMAP:
		case CHNK_HEAP:
			if (!data_read)
			{
   			    ckptRestoreMmap(0);
			    ckptSystemIgnore++;
			    data_read = 1;
			}
			ckptRestoreData(&image, size, start);
			break;
		case CHNK_DATA:
			ckptRestoreData(&image, size, start);
			break;
		case CHNK_STACK:
			if (!data_read)
			{
    			    ckptRestoreMmap(0);
			    ckptSystemIgnore++;
			    data_read = 1;
			}
#ifdef WITH_MPICH			
		        ckptMpichInsideRestore();
			ckptMpichAfterRestore();
#endif
			ckptRestoreStack(&image, size, stack_start);
			break;
		case CHNK_MPICH:
			if (!data_read)
			{
    			    ckptRestoreMmap(0);
			    ckptSystemIgnore++;
			    data_read = 1;
			}
#ifdef WITH_MPICH			
			ckptMpichRestoreConvert(&start);
#endif
			ckptRestoreData(&image, size, start);
			break;
		default:
			ckptLog(CKPT_LOG_WARNING, "Unknown type %d, skipping over", type);
			ckptReadChunkSkip(&image);
		}
	}

	/* This point should never be reached, control is passed on last chunk, CHNK_STACK */
	abort();
}

/* Restore checkpoint header */
int 
ckptRestoreHeader(ckptImage * image, size_t size, void **stack_start)
{
	struct ckptCheckpointHdr header;

	ckptReadChunkData(image, &header);

	if (header.np != CKPT_NP)
	{
		ckptLog(CKPT_LOG_ERROR, "Number of processes mismatch: was %d, now %d", header.np, CKPT_NUMBER);
		abort();
	}

	if (!ckptDisableMtimeCheck && header.mtime != ckptElfGetMtime())
	{
		ckptLog(CKPT_LOG_ERROR, "Wrong program image file (%lu != %lu)", header.mtime, ckptElfGetMtime());
		abort();
	}

	if (brk((void *)header.brk) != 0)
	{
		ckptLog(CKPT_LOG_ERROR, "Unable to set new break @%p", header.brk);
		abort();
	}
	*stack_start = (void *)header.stack_start;

	return 1;
}

/* Restore checkpoint data */
int 
ckptRestoreData(ckptImage * image, size_t size, void *start)
{
	/* Save vital data pointer, which should revive restoring data segment */
	char **vital_save = ckptVitalSave();
	char *d_vital_save=  *vital_save;

	if (!ckptReadChunkData(image, start))
	{
		ckptLog(CKPT_LOG_DEBUG, "Error restoring data: start %x", start);
		abort();
	}

	/* Restore vital data pointer */
	*vital_save = d_vital_save;
	
	return 1;
}

int 
ckptRestoreStack(ckptImage * image, size_t size, void *stack_start)
{
	int             dummy;

	if ((void *)&dummy > (void *)((char *)stack_start - size - 2*STACK_GAP))
		ckptRestoreStack(image, size, stack_start);

	/* Here we do some hack, reading directly from file, but there's no
	   other way here */
	dummy = image->fd;
	F_REAL_READ(image->fd, (void *)((char *)stack_start - size - STACK_GAP), size);
	F_REAL_CLOSE(dummy);

	ckptSystemIgnore--;

	siglongjmp(ckptJumpBuffer, 1);

	/* XXX: to make compiler happy */
	return 0;
}
