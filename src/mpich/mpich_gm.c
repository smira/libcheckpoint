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
 *  Interface to checkpointing with MPICH 
 *  library, device ch_gm
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>

#include "config.h"

#include "mpich_gm_sync.h"
#include "mpich_gm_exclude.h"
#include "../system.h"
#include "../debug.h"
#include "../areas.h"
#include "../image.h"

int ckptGMmyproc;
int ckptGMnumprocs;

/* Initialise MPICH, before actual checkpoiting begins */
int 
ckptMpichInit(int *argc, char ***argv)
{
	void *dummy;

	/* Initialize malloc before MPI initialization */
	dummy = malloc(65536);

	/* While MPI is initializing, we block all interceptions */
	ckptSystemIgnore++;
	ckptMallocExclude++;

	MPI_Init(argc, argv);

	/* After initialization, we are ready again */
	ckptMallocExclude--;
	ckptSystemIgnore--;

	MPI_Comm_size(MPI_COMM_WORLD, &ckptGMnumprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &ckptGMmyproc);

	if (!ckptGMSyncInit())
		return 0;
	
	ckptGMExcludeOnce();

	return 1;
}

/* Finalise MPICH */
int
ckptMpichDone()
{
	ckptGMSyncDone();
	return MPI_Finalize();
}

/* Get node number */
int 
ckptMpichNumber()
{
	return ckptGMmyproc;
}

/* Get total number of processes */
int 
ckptMpichNP()
{
	return ckptGMnumprocs;
}

extern int ckptCheckRestore();

/* Decisions:
 * 'C' - checkpoint
 * 'U' - run (continue without restore)
 * 'R' - restore
 * 'S' - just sync
 * 'E' - there wasn't agreement on processes decisions
 * 'A' - there was error in sync
 */


/* Checks whether we should restore now */
int 
ckptMpichCheckRestore()
{
	char my_decision = (ckptCheckRestore() ? 'R' : 'U');
	char decision;

	decision = ckptGMSyncDecision(my_decision, 'R');
	switch (decision)
	{
	case 'R':
		return 1;
	case 'U':
	case 'E':
		return 0;
	case 'A':
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed to find out decision!");
		abort();
	}
}


/***** HOOKS *********/
extern void 
ckptPrintMemAreas();

void 
ckptMpichBeforeCheckpoint()
{
	ckptGMExcludeBeforeCheckpoint();

	switch (ckptGMSyncDecision('S', 'S'))
	{
	case 'S':
		return;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichBeforeCheckpoint!");
		abort();
	}
}

void 
ckptMpichAfterCheckpoint()
{
	switch (ckptGMSyncDecision('S', 'S'))
	{
	case 'S':
		return;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichAfterCheckpoint!");
		abort();
	}
}

void  
ckptMpichBeforeRestore() 
{ 
	switch (ckptGMSyncDecision('S', 'S'))
	{
	case 'S':
		return;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichBeforeRestore!");
		abort();
	}
}

void 
ckptMpichInsideRestore()
{
}

void 
ckptMpichAfterRestore()
{
	switch (ckptGMSyncDecision('S', 'S'))
	{
	case 'S':
		return;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichAfterRestore!");
		abort();
	}
}

void 
ckptMpichRestoreConvert(void **start)
{
}


