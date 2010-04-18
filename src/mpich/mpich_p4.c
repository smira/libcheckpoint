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
 *  library, device ch_p4
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>

#include "config.h"
#include "mpich_p4_sync.h"
#include "../debug.h"
#include "../areas.h"
#include "../system.h"
#include "../vital.h"

#include "p4.h"
#include "p4_globals.h"
#include "p4_defs.h"

int ckptP4myproc;
int ckptP4numprocs;

extern void 
ckptExcludeMem(void *start, size_t size);


/* Initialise MPICH, before actual checkpoiting begins */
int 
ckptMpichInit(int *argc, char ***argv)
{
	MPI_Init(argc, argv);

	MPI_Comm_size(MPI_COMM_WORLD, &ckptP4numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &ckptP4myproc);

	if (!ckptP4SyncInit())
		return 0;
	
	return 1;
}

/* Finalise MPICH */
int
ckptMpichDone()
{
        extern void ckptMPIOnlineFinalize();

        //ckptMPIOnlineFinalize();
	ckptP4SyncDone();
	return MPI_Finalize();
}

/* Get node number */
int 
ckptMpichNumber()
{
	return ckptP4myproc;
}

/* Get total number of processes */
int 
ckptMpichNP()
{
	return ckptP4numprocs;
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

	decision = ckptP4SyncDecision(my_decision, 'R');
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

#define VITAL_EXECER_PG		0x0001000
#define VITAL_GLOBAL_PROC_INFO	0x0002001
#define VITAL_GLOBAL_LISTENER	0x0002002
#define VITAL_GLOBAL_HOSTNAME	0x0002003
#define VITAL_GLOBAL_DESTID	0x0002004
#define VITAL_GLOBAL_APPID	0x0002005
#define VITAL_LOCAL_LISTENER	0x0003001
#define VITAL_LOCAL_CONNTAB	0x0003002
#define VITAL_LOCAL_PROCGROUP	0x0003003
#define VITAL_LISTENER_FD	0x0004001
#define VITAL_LISTENER_SL_PID	0x0004002
#define VITAL_LISTENER_SL_FD	0x0004003

extern void 
ckptPrintMemAreas();

static void
ckptMpichExclude();

int 
ckptMpichBeforeCheckpoint()
{
        char decision;

        ckptLog(CKPT_LOG_DEBUG, "Entering sync, ckptMpichBeforeCheckpoint");

	switch (decision = ckptP4SyncDecision('S', 'S'))
	{
	case 'S':
		ckptMpichExclude();
                ckptLog(CKPT_LOG_DEBUG, "Leaving sync, ckptMpichBeforeCheckpoint");
		return 1;
        case 'E':
                ckptLog(CKPT_LOG_DEBUG, "Leaving sync error, ckptMpichBeforeCheckpoint");
                return 0;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichBeforeCheckpoint: %c, expected %c!", decision, 'S');
		abort();
	}
}

void 
ckptMpichAfterCheckpoint()
{
        char decision;

        ckptLog(CKPT_LOG_DEBUG, "Entering sync, ckptMpichAfterCheckpoint");

	switch (decision = ckptP4SyncDecision('S', 'S'))
	{
	case 'S':
                ckptLog(CKPT_LOG_DEBUG, "Leaving sync, ckptMpichAfterCheckpoint");
		return;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichAfterCheckpoint: %c, expected %c!", decision, 'S');
		abort();
	}
}

void  
ckptMpichBeforeRestore() 
{ 
	switch (ckptP4SyncDecision('S', 'S'))
	{
	case 'S':
		break;
	default:
		ckptLog(CKPT_LOG_ERROR, "Failed sync: ckptMpichBeforeRestore!");
		abort();
	}

	ckptVitalStackInit();

	if (execer_pg != NULL)
		ckptVitalPush(VITAL_EXECER_PG, sizeof(struct p4_procgroup), execer_pg);
	ckptVitalPush(VITAL_GLOBAL_PROC_INFO, sizeof(p4_global->proctable), p4_global->proctable);
	ckptVitalPush(VITAL_GLOBAL_LISTENER, sizeof(int), &p4_global->listener_pid);
	ckptVitalPush(VITAL_GLOBAL_LISTENER, sizeof(int), &p4_global->listener_port);
	ckptVitalPush(VITAL_GLOBAL_LISTENER, sizeof(int), &p4_global->listener_fd);
	ckptVitalPush(VITAL_GLOBAL_HOSTNAME, sizeof(p4_global->my_host_name), p4_global->my_host_name);
	ckptVitalPush(VITAL_GLOBAL_DESTID, sizeof(p4_global->dest_id), p4_global->dest_id);
	ckptVitalPush(VITAL_GLOBAL_APPID, sizeof(p4_global->application_id), p4_global->application_id);
	ckptVitalPush(VITAL_LOCAL_LISTENER, sizeof(int), &p4_local->listener_fd);
	ckptVitalPush(VITAL_LOCAL_CONNTAB, sizeof(struct connection)*p4_global->num_in_proctable, p4_local->conntab);
	if (p4_local->procgroup)
		ckptVitalPush(VITAL_LOCAL_PROCGROUP, sizeof(struct p4_procgroup), p4_local->procgroup);
	ckptVitalPush(VITAL_LOCAL_LISTENER, sizeof(int), &p4_local->my_id);
	ckptVitalPush(VITAL_LOCAL_LISTENER, sizeof(P4BOOL), &p4_local->am_bm);
	ckptVitalPush(VITAL_LOCAL_LISTENER, sizeof(int), &p4_local->local_commtype);
	ckptVitalPush(VITAL_LISTENER_FD, sizeof(int), &listener_info->listening_fd);
	ckptVitalPush(VITAL_LISTENER_SL_PID, sizeof(*listener_info->slave_fd)*listener_info->num, listener_info->slave_fd);
	ckptVitalPush(VITAL_LISTENER_SL_FD, sizeof(*listener_info->slave_fd)*listener_info->num, listener_info->slave_pid);
}

void 
ckptMpichInsideRestore()
{
}

void 
ckptMpichAfterRestore()
{
	ckptVitalStackInit();

	if (execer_pg != NULL)
		ckptVitalPop(VITAL_EXECER_PG, sizeof(struct p4_procgroup), execer_pg);
	ckptVitalPop(VITAL_GLOBAL_PROC_INFO, sizeof(p4_global->proctable), p4_global->proctable);
	ckptVitalPop(VITAL_GLOBAL_LISTENER, sizeof(int), &p4_global->listener_pid);
	ckptVitalPop(VITAL_GLOBAL_LISTENER, sizeof(int), &p4_global->listener_port);
	ckptVitalPop(VITAL_GLOBAL_LISTENER, sizeof(int), &p4_global->listener_fd);
	ckptVitalPop(VITAL_GLOBAL_HOSTNAME, sizeof(p4_global->my_host_name), p4_global->my_host_name);
	ckptVitalPop(VITAL_GLOBAL_DESTID, sizeof(p4_global->dest_id), p4_global->dest_id);
	ckptVitalPop(VITAL_GLOBAL_APPID, sizeof(p4_global->application_id), p4_global->application_id);
	ckptVitalPop(VITAL_LOCAL_LISTENER, sizeof(int), &p4_local->listener_fd);
	ckptVitalPop(VITAL_LOCAL_CONNTAB, sizeof(struct connection)*p4_global->num_in_proctable, p4_local->conntab);
	if (p4_local->procgroup)
		ckptVitalPop(VITAL_LOCAL_PROCGROUP, sizeof(struct p4_procgroup), p4_local->procgroup);
	ckptVitalPop(VITAL_LOCAL_LISTENER, sizeof(int), &p4_local->my_id);
	ckptVitalPop(VITAL_LOCAL_LISTENER, sizeof(P4BOOL), &p4_local->am_bm);
	ckptVitalPop(VITAL_LOCAL_LISTENER, sizeof(int), &p4_local->local_commtype);
	ckptVitalPop(VITAL_LISTENER_FD, sizeof(int), &listener_info->listening_fd);
	ckptVitalPop(VITAL_LISTENER_SL_PID, sizeof(*listener_info->slave_fd)*listener_info->num, listener_info->slave_fd);
	ckptVitalPop(VITAL_LISTENER_SL_FD, sizeof(*listener_info->slave_fd)*listener_info->num, listener_info->slave_pid);

	switch (ckptP4SyncDecision('S', 'S'))
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


/*** EXCLUDE ***/

#define EXCLUDE_MEM(p, s, m)			\
		ckptExcludeMem(p, s);		\
		/*ckptLog(CKPT_LOG_ERROR, "EXCLUDE: %p, %u [%s]\n", p, s, m); */

static void
ckptMpichExclude()
{
	/*
	 * We should EXCLUDE MEMORY only for static regions of memory.
	 * Dynamic memory contents should be saved to VITAL area and restored after 
	 * checkpoint restore finish
	 */

	/* p4_global.h */
	EXCLUDE_MEM(procgroup_file, sizeof(procgroup_file), "procgroup_file");
	EXCLUDE_MEM(bm_outfile, sizeof(bm_outfile), "bm_outfile");
	EXCLUDE_MEM(rm_outfile_head, sizeof(rm_outfile_head), "rm_outfile_head");
	EXCLUDE_MEM(whoami_p4, sizeof(whoami_p4), "whoami_p4");
	EXCLUDE_MEM(&p4_debug_level, sizeof(p4_debug_level), "p4_debug_level");
	EXCLUDE_MEM(&p4_remote_debug_level, sizeof(p4_remote_debug_level), "p4_remote_debug_level");
	EXCLUDE_MEM(p4_wd, sizeof(p4_wd), "p4_wd");
	EXCLUDE_MEM(p4_myname_in_procgroup, sizeof(p4_myname_in_procgroup), "p4_myname_in_procgroup");
	EXCLUDE_MEM(&p4_rm_rank, sizeof(p4_rm_rank), "p4_rm_rank");
	EXCLUDE_MEM(&logging_flag, sizeof(logging_flag), "logging_flag");
	EXCLUDE_MEM(&execer_mynodenum, sizeof(execer_mynodenum), "execer_mynodenum");
	EXCLUDE_MEM(execer_id, sizeof(execer_id), "execer_id");
	EXCLUDE_MEM(execer_myhost, sizeof(execer_myhost), "execer_myhost");
	EXCLUDE_MEM(&execer_mynumprocs, sizeof(execer_mynumprocs), "execer_mynumprocs");
	EXCLUDE_MEM(execer_masthost, sizeof(execer_masthost), "execer_masthost");
#ifdef OLD_EXECER
	EXCLUDE_MEM(execer_jobname, sizeof(execer_jobname), "execer_jobname");
#endif	
	EXCLUDE_MEM(&execer_mastport, sizeof(execer_mastport), "execer_mastport");
	EXCLUDE_MEM(&execer_numtotnodes, sizeof(execer_numtotnodes), "execer_numtotnodes");
	EXCLUDE_MEM(&execer_starting_remotes, sizeof(execer_starting_remotes), "execer_starting_remotes");
	EXCLUDE_MEM(local_domain, sizeof(local_domain), "local_domain");
	EXCLUDE_MEM(&globmemsize, sizeof(globmemsize), "globmemsize");
	EXCLUDE_MEM(&sserver_port, sizeof(sserver_port), "sserver_port");
	EXCLUDE_MEM(&hand_start_remotes, sizeof(hand_start_remotes), "hand_start_remotess");

	/* p4_defs.h */
	EXCLUDE_MEM(&p4_brdcst_info, sizeof(struct p4_brdcst_info_struct), "p4_brdcst_info");
	
}

