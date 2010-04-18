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
 *  Excluding memory areas associated with
 *  mpich-gm
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "../areas.h"
#include "../debug.h"
#include "mpich_gm.h"

#include "gm.h"
#include "gm_internal.h"
#include "gm_lanai.h"
#include "gm_types.h"
#include "gm_zone_types.h"

/*
 * External definitions from mpich_gm & gm
 */

struct gmpi_send_buf
{
  unsigned int type;
  unsigned int dest;
  unsigned int length;
  unsigned int register_length;
  void * source_ptr;
  void * target_ptr;
  /*MPIR_SHANDLE*/ void * shandle;
  struct gmpi_send_buf * next;
  unsigned int buffer; 
};


struct gmpi_var
{
  unsigned int my_global_node_id;
  unsigned int send_tokens;
  unsigned int max_send_tokens;
  unsigned int recv_tokens;
  unsigned int max_recv_tokens;
  unsigned int send_buf_fifo_queued;
  unsigned int send_dma_buf_pool_remaining;
  unsigned int malloc_send_buf_allocated;
  unsigned int pending_put_callback;
  unsigned int pending_send_callback;
  unsigned int unexpected_short;
  unsigned int malloc_hook_flag;
  unsigned int eager_size;
  unsigned int shmem;
  unsigned int magic;
  unsigned int mpd;
  char my_hostname[256];
  struct sockaddr_in master_addr;
  struct sockaddr_in slave_addr;
  /*FILE*/void * debug_output_filedesc;
  /*union gm_recv_event *(*gm_receive_mode)(struct gm_port *);*/
  void (*gm_receive_mode)(void);

  struct gmpi_send_buf * send_buf_fifo_head;
  struct gmpi_send_buf * send_buf_fifo_tail;
  struct gmpi_send_buf * send_dma_buf_free;

  unsigned int *global_node_ids;  /* Global node ids */
  unsigned int *local_node_ids;   /* Local node ids */
  char **host_names;           /* names of machines */
  char **exec_names;           /* names of executables */
  unsigned int *numa_nodes;    /* numa node numbers */
  unsigned int *mpi_pids;      /* PID of remote processes */
  unsigned int *port_ids;      /* port ids */
  unsigned int *board_ids;     /* board/unit number */
  unsigned int *pending_sends; /* pending sends (timeout) */
  unsigned int *dropped_sends; /* dropped sends (timeout) */
};

extern struct gmpi_var gmpi;

#define SMPI_MAX_NUMNODES 8192

struct smpi_var {
  void * mmap_ptr;
  /*struct smpi_send_fifo_req*/ void * send_fifo_head;
  /*struct smpi_send_fifo_req*/ void * send_fifo_tail;
  /*struct gm_lookaside*/ void * send_fifo_lookaside;
  unsigned int send_fifo_queued;
  unsigned int malloc_send_buf_allocated;
  unsigned int my_local_id;
  unsigned int num_local_nodes;
  unsigned int local_nodes[SMPI_MAX_NUMNODES];
  int available_queue_length;
  int pending;
  int fd;
};

extern struct smpi_var smpi;

#define GM_MAX_HOST_NAME_LEN 	128 
#define GM_MAX_EXECNAME_LEN	256

#define GMPI_DMA_SEND_BUF 2
#define GMPI_MALLOC_SEND_BUF 4
#define GMPI_PUT_DATA 8

extern struct gm_port *gmpi_gm_port;
/*
 * End of external definitions
 */

/* This function is called after initialization */
void
ckptGMExcludeOnce()
{
}

#define EXCLUDE_MEM(p, s, m)			\
		ckptExcludeMem(p, s);		\
		ckptLog(CKPT_LOG_ERROR, "EXCLUDE: %p, %x [%s]\n", p, s, m); 

/* This function is called every time checkpoint is taken */
void
ckptGMExcludeBeforeCheckpoint()
{
	struct gmpi_send_buf *p;
	int i;

	EXCLUDE_MEM(&gmpi, sizeof(gmpi), "gmpi");
	EXCLUDE_MEM(gmpi.global_node_ids, 	ckptGMnumprocs*sizeof(unsigned int), "gmpi.global_node_ids");
	EXCLUDE_MEM(gmpi.local_node_ids, 	ckptGMnumprocs*sizeof(unsigned int), "gmpi.local_node_ids");
	EXCLUDE_MEM(gmpi.mpi_pids, 		ckptGMnumprocs*sizeof(unsigned int), "gmpi.mpi_pids");
	EXCLUDE_MEM(gmpi.port_ids, 		ckptGMnumprocs*sizeof(unsigned int), "gmpi.port_ids");
	EXCLUDE_MEM(gmpi.board_ids, 		ckptGMnumprocs*sizeof(unsigned int), "gmpi.board_ids");
	EXCLUDE_MEM(gmpi.pending_sends, 	ckptGMnumprocs*sizeof(unsigned int), "gmpi.pending_sends");
	EXCLUDE_MEM(gmpi.dropped_sends, 	ckptGMnumprocs*sizeof(unsigned int), "gmpi.dropped_sends");
	EXCLUDE_MEM(gmpi.host_names, 		ckptGMnumprocs*sizeof(char *), 	     "gmpi.host_names");
	EXCLUDE_MEM(gmpi.exec_names, 		ckptGMnumprocs*sizeof(char *),       "gmpi.exec_names");
	EXCLUDE_MEM(gmpi.numa_nodes, 		ckptGMnumprocs*sizeof(unsigned int), "gmpi.numa_nodes");

	if (ckptGMmyproc == 0)
	{
		int j;

		for (j = 0; j < ckptGMnumprocs; j++)
		{
			EXCLUDE_MEM(gmpi.host_names[j], strlen(gmpi.host_names[j]) + 1, "gmpi.host_names[j]");
			EXCLUDE_MEM(gmpi.exec_names[j], strlen(gmpi.exec_names[j]) + 1, "gmpi.exec_names[j]");
		}
	}
	else
	{
		EXCLUDE_MEM(gmpi.host_names[ckptGMmyproc], strlen(gmpi.host_names[ckptGMmyproc]) + 1, "gmpi.host_names[me]");
		EXCLUDE_MEM(gmpi.exec_names[ckptGMmyproc], strlen(gmpi.exec_names[ckptGMmyproc]) + 1, "gmpi.exec_names[me]");
	}
	
	/*for (p = gmpi.send_dma_buf_free; p != NULL; p = p->next)
	{
		if (p->type == GMPI_DMA_SEND_BUF)
		{
			EXCLUDE_MEM(p, p->length + sizeof(struct gmpi_send_buf) - sizeof(unsigned int), "gmpi.dma_buf");
		}
		else			
		{
			EXCLUDE_MEM(p, sizeof(struct gmpi_send_buf), "gmpi.other_buf");
		}
	}*/

	EXCLUDE_MEM(&smpi, sizeof(smpi), "smpi");

	EXCLUDE_MEM(gmpi_gm_port, sizeof(struct gm_port), "gmpi_gm_port");

	EXCLUDE_MEM(gmpi_gm_port->recv_queue_allocation, (GM_NUM_RECV_QUEUE_SLOTS+1)*sizeof (gm_recv_queue_slot_t), 
			"gmpi_gm_port->recv_queue_allocation");

	EXCLUDE_MEM(gmpi_gm_port->lanai, sizeof (struct gm_port_unprotected_lanai_side), 
			"gmpi_gm_port->lanai");

	EXCLUDE_MEM(gmpi_gm_port->mappings, sizeof (struct gm_mapping_specs), 
			"gmpi_gm_port->mappings");


	for (i = 0; i < 32; i++)
	{
		EXCLUDE_MEM(gmpi_gm_port->dma.zone[i], sizeof(struct gm_zone), 
				"gmpi_gm_port->gm_zone[i]");
	}
}

