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
 *  Synchronization of all processes
 *  mpich-gm
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include <mpi.h>

#include "config.h"

#include "mpich_gm.h"
#include "../system.h"
#include "../areas.h"
#include "../debug.h"
#include "../benchmark.h"

static struct 
{
	int listen_socket;
	int com_socket;
	int sync_sockets[MAX_PROCS];
} global;

/* Forward declarations */
static int ckptGMSyncWrite(int socket, char ch);
static int ckptGMSyncRead(int socket, char *ch);
extern void ckptExcludeMem(void *start, size_t size);

/* Initialize synchronization subsystem */
int ckptGMSyncInit()
{
	struct sockaddr_in addr;
	int port;

	if (ckptGMmyproc == 0)
	{
		/* Initialize listening socket */
		char hostname[256];
		struct hostent *host;

		/* Find out IP address of current host */
		if (gethostname(hostname, sizeof(hostname)) != 0)
		{
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to get hostname");
			return 0;
		}

		host = gethostbyname(hostname);
		if (host == NULL || host->h_addrtype != AF_INET)
		{
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to resolve hostname");
			return 0;
		}

		addr.sin_family = AF_INET;
		memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);

		global.listen_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (global.listen_socket == -1)
		{
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable get socket");
			return 0;
		}

		/* Try to find out free port number between 2000 and 5000 */
		for (port = 2000; port < 5000; port++)
		{
			addr.sin_port = htons(port);
			if (bind(global.listen_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			{
				if (errno == EADDRINUSE)
					continue;
				F_REAL_CLOSE(global.listen_socket);
				ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to bind listening socket");
				return 0;
			}
			break;
		}

		if (port == 5000)
		{
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to bind listening socket (no ports available)");
			F_REAL_CLOSE(global.listen_socket);
			return 0;
		}

		if (listen(global.listen_socket, 5) == -1)
		{
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to listen socket");
			F_REAL_CLOSE(global.listen_socket);
			return 0;
		}
	}

	/* Send address of root process (#0) to everyone */
	MPI_Bcast(&addr.sin_addr.s_addr, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&port, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	/* Everyone except root connects to root */
	if (ckptGMmyproc == 0)
	{
		int n;

		for (n = 0; n < ckptGMnumprocs-1; n++)
		{
retry:
			global.sync_sockets[n] = accept(global.listen_socket, (struct sockaddr *)NULL, (socklen_t *)NULL);
			if (global.sync_sockets[n] == -1)
			{
				if(errno == EINTR)
					goto retry;
				else
				{
					ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to get children's sockets");
					F_REAL_CLOSE(global.listen_socket);
					return 0;
				}
			}
		}
	}
	else
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		global.com_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (global.com_socket == -1)
		{
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to get socket");
			return 0;
		}

		if (connect(global.com_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		{
			F_REAL_CLOSE(global.com_socket);
			ckptLog(CKPT_LOG_ERROR, "ckptGMSyncInit(): Unable to connect to root");
			return 0;
		}
	}
	
	/* Exclude socket area from checkpointing */
	ckptExcludeMem(&global, sizeof(global));
		
	/* Everything is OK, all sockets connected */
	return 1;
}

/* Finalize synchronization subsystem */
int ckptGMSyncDone()
{
	if (ckptGMmyproc == 0)
	{
		int n;

		F_REAL_CLOSE(global.listen_socket);
		for (n = 0; n < ckptGMnumprocs-1; n++)
			F_REAL_CLOSE(global.sync_sockets[n]);
	}
	else
		F_REAL_CLOSE(global.com_socket);

	return 1;
}

/* Find out decision
 * my_decision - what this process decided
 * req_decision - "good decision"
 * returns cooperative decision of all processes:
 *   if everyone gives req_decision, returns req_decision
 *   else returns 'E'
 *   on communication error return 'A'
 */

char ckptGMSyncDecision(char my_decision, char req_decision)
{
	char result;

	CKPT_BENCHMARK(CKPT_BNCH_SYNC_BEGIN);

	if (ckptGMmyproc == 0)
	{
		/* Read everyone's decision */
		int n;
		char decisions[MAX_PROCS];

		for (n = 0; n < ckptGMnumprocs-1; n++)
			if (!ckptGMSyncRead(global.sync_sockets[n], decisions+n))
				return 'A';
		decisions[ckptGMnumprocs-1] = my_decision;

		/* Analyze decisions */
		result = req_decision;
		for (n = 0; n < ckptGMnumprocs; n++)
			if (decisions[n] != req_decision)
			{
				result = 'E';
				break;
			}
	
		/* Send decision to everyone */
		for (n = 0; n < ckptGMnumprocs-1; n++)
			if (!ckptGMSyncWrite(global.sync_sockets[n], result))
				return 'A';

	}
	else
	{
		/* Send my decision */
		if (!ckptGMSyncWrite(global.com_socket, my_decision))
			return 'A';
		/* Find out result */
		if (!ckptGMSyncRead(global.com_socket, &result))
			return 'A';
	}

	CKPT_BENCHMARK(CKPT_BNCH_SYNC_END);

	return result;
}

/* Safe read & write, non-interruptible */
static int ckptGMSyncWrite(int socket, char ch)
{
	while (F_REAL_WRITE(socket, &ch, 1) == -1)
	{
		if (errno != EINTR)
			return 0;
	}
	return 1;
}

static int ckptGMSyncRead(int socket, char *ch)
{
	while (F_REAL_READ(socket, ch, 1) == -1)
	{
		if (errno != EINTR)
			return 0;
	}
	return 1;
}


