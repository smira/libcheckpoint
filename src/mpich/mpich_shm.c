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
 *  library, device ch_shm
 *  NB: Requires a patch from patches/.
 *
 */

#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "config.h"

#include <mpi.h>

#include "mpich.h"
#include "../areas.h"
#include "../image.h"
#include "../system.h"
#include "../vital.h"
#include "../benchmark.h"

/* Number of nodes and my node number */
static int numprocs;
static int myproc;

/* Synchronization semaphore */
static int ckpt_sem = -1;

/* These variables are exported from libmpich, file mpid/ch_shmem/p2p.c 
   See also patches/mpich/ch_shmem.diff 
*/
extern int sysv_num_shmids;
extern int sysv_shmid[];
extern void *sysv_shmat[];

/* sysv_semid0 holds 1st semaphore allocated by MPICH to store locks */

extern int sysv_semid0;

/* This is a hack to to get to struct p2_global_data *p2_global from
   from mpid/ch_shmem/p2psemop.c
*/

extern struct 
{
	int sem_num;
	int sem_id[1];
} *p2_global;

static void ckptMpichSynchronizeAll();

/* Initialise MPICH, before actual checkpoiting begins */
int 
ckptMpichInit(int *argc, char ***argv)
{
	/* initialize semaphore to use with checkpointing */
	ckpt_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | SEM_R | SEM_A);
	
	MPI_Init(argc, argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myproc);
	
	return 1;
}

/* Finalise MPICH */
int
ckptMpichDone()
{
	/* Delete synchronization semaphore */
	semctl(ckpt_sem, 1, IPC_RMID);
	return MPI_Finalize();
}

/* Get node number */
int 
ckptMpichNumber()
{
	return myproc;
}


/* Get total number of processes */
int 
ckptMpichNP()
{
	return numprocs;
}

extern int ckptCheckRestore();

/* Checks whether we should restore now */
int 
ckptMpichCheckRestore()
{
	return ckptCheckRestore();
}

/* Hooks from main code to support checkpointing with MPICH */
void 
ckptMpichBeforeCheckpoint()
{
	int i;
	struct shmid_ds shm_info;
	
	if (myproc == 0)
	{
#if 0
	    printf("Number of shm segments: %d\n", sysv_num_shmids);
#endif	
	    for(i = 0; i < sysv_num_shmids; i++)
	    {
		shmctl(sysv_shmid[i], IPC_STAT, &shm_info);
#if 0
		printf("Segment id %d, addr %p, size %d\n", sysv_shmid[i], sysv_shmat[i], shm_info.shm_segsz);
#endif
		ckptIncludeMemArea(sysv_shmat[i], shm_info.shm_segsz, CHNK_MPICH);
	    }
	}

	/* Synchronize to begin checkpointing synchronously in all processes */
	MPI_Barrier(MPI_COMM_WORLD);
}

void 
ckptMpichAfterCheckpoint()
{
	ckptMpichSynchronizeAll();
}

/* Position in sysv_shmat, which was restored */
static int sysv_shmpos;

void  
ckptMpichBeforeRestore() 
{ 
	sysv_shmpos = 0;
	if (myproc == 0) 	
	{ 	    
	    int offset = 0;
	    
	    /* Save our semaphore id and shared memory ids in vital area */
	    ckptVitalPut(offset, &ckpt_sem, sizeof(ckpt_sem));
	    offset += sizeof(ckpt_sem);
	    ckptVitalPut(offset, &sysv_num_shmids, sizeof(sysv_num_shmids));
	    offset += sizeof(sysv_num_shmids);
	    ckptVitalPut(offset, sysv_shmid, sizeof(int)*sysv_num_shmids);
	    offset += sizeof(int)*sysv_num_shmids;
	    ckptVitalPut(offset, sysv_shmat, sizeof(void *)*sysv_num_shmids);
	    offset += sizeof(void *)*sysv_num_shmids;
	    ckptVitalPut(offset, &sysv_semid0, sizeof(sysv_semid0));
	    offset += sizeof(sysv_semid0);
	    ckptVitalPut(offset, &p2_global->sem_num, sizeof(int));
	    offset += sizeof(int);
	    ckptVitalPut(offset, p2_global->sem_id, sizeof(int)*p2_global->sem_num);
	    offset += sizeof(int)*p2_global->sem_num;
	}
	ckptMpichSynchronizeAll();
}

void 
ckptMpichInsideRestore()
{
	int offset = 0;
	    
	/* Restore our semaphore id and shared memory ids from vital area */
	ckptVitalGet(offset, &ckpt_sem, sizeof(ckpt_sem));
	offset += sizeof(ckpt_sem);
	ckptVitalGet(offset, &sysv_num_shmids, sizeof(sysv_num_shmids));
	offset += sizeof(sysv_num_shmids);
	ckptVitalGet(offset, sysv_shmid, sizeof(int)*sysv_num_shmids);
	offset += sizeof(int)*sysv_num_shmids;
	ckptVitalGet(offset, sysv_shmat, sizeof(void *)*sysv_num_shmids);
	offset += sizeof(void *)*sysv_num_shmids;
	ckptVitalGet(offset, &sysv_semid0, sizeof(sysv_semid0));
	offset += sizeof(sysv_semid0);
	ckptVitalGet(offset, &p2_global->sem_num, sizeof(int));
	offset += sizeof(int);
	ckptVitalGet(offset, p2_global->sem_id, sizeof(int)*p2_global->sem_num);
	offset += sizeof(int)*p2_global->sem_num;
}

void 
ckptMpichAfterRestore()
{
	/*printf("bla-bla %d\n");*/
	ckptMpichSynchronizeAll();
}

void 
ckptMpichRestoreConvert(void **start)
{
	*start = sysv_shmat[sysv_shmpos++];
}

/* Synchronize all processes */
static void
ckptMpichSynchronizeAll()
{
	CKPT_BENCHMARK(CKPT_BNCH_SYNC_BEGIN);

	if (numprocs > 1)
	{
	    struct sembuf sb;
	    
	    /*printf("%d: Synchronizing (%d)...\n", myproc, numprocs);*/

	    sb.sem_num = 0;
	    sb.sem_flg = 0;
	    
	    if (myproc == 0)
	    {
		sb.sem_op = 0;
		semop(ckpt_sem, &sb, 1);
		sb.sem_op = (numprocs-1);
		semop(ckpt_sem, &sb, 1);
		sb.sem_op = 0;
		semop(ckpt_sem, &sb, 1);
	    }
	    else
	    {
		sb.sem_op = -1;
		semop(ckpt_sem, &sb, 1);
	    }
	    
 	    /*printf("%d: Done synchronizing...\n", myproc);*/

	}	

	CKPT_BENCHMARK(CKPT_BNCH_SYNC_END);
}

