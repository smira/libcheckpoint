/*-
 * Copyright (c) 2002-2007 Andrey Smirnov.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 */

/**
 * \file checkpoint.c
 * �������� ������, ������ ����������� �����.
 */

/**
 * \mainpage ���������� libcheckpoint
 *
 * ���������� ��������� ��������� ����������� ����� ���������������� � ������������ (������������ ���������� MPICH) �����.
 *
 * \section features �������� ��������������
  *
 * � �������� �������������� ���������� ����� �������:
 *     - ���������� �������������� ����������� ����� �� ������ �������� ������������.
 *     - ��������� ��������� ����������:
 *        - FreeBSD/x86
 *        - Linux/x86
 *        - Linux/Alpha 
 *     - �������������� ����� ����������������:
 *        - C/C++
 *        - Fortran 
 *     - ����������� ����� ��� ������������ �����, ������������ ���������� MPICH:
 *        - � ���������������� ����� Myrinet (MPICH-GM);
 *        - � ���������������� ����� Ethernet (ch_p4)
 *        - � ����������� ������� (SMP). 
 *     - ���������� ��������� ��������:
 *        - ������� ������;
 *        - ����;
 *        - ���� (malloc/free);
 *        - �������� �������� (��������� ��������� ����������). 
 *     - ���������� ��������� ������������ �������:
 *        - ����������� � ������ (mmap/munmap);
 *        - �������� ����� (open/close/lseek/read/write/...);
 *        - ����������� ��������. 
 *     - ������ ��������� �������������:
 *        - ��������� ����������� ����������� �����;
 *        - ������ ������ ����������� �����;
 *        - ���� ����������� ����� ��� ����������������. 
 *     - ��������� ���������� ���������� �� ����� ���������� (���������������� ���� ��� ���������� ���������).
 *     - �������� ��������� ����� �������� � ���������� ������������.
 *     - ��������� ���������� �� ������������� ������ ����������.
 *
 * \section setup ��������� ����������
 *
 * \subsection compile ������ � ���������
 *
 *  � ������ ������� ����� ������ ������� ������ � ��������� ���������� \c libcheckpoint. ��� ��������� ���������� ��������� 
 *  ��������� ��������:
 *
 * <tt>tar xzf libcheckpoint-2.x.tar.gz \n
 * cd libcheckpoint-2.x/ \n
 * ./configure ... \n
 * make all \n
 * make install</tt>
 *
 * \subsection configure ��������� configure
 *
 *   - \c --with-mpich-dev=���������� \n
 *     �������� ���������� ���������� MPICH, �� ������� ������ ���� ��������� ����������. 
 *     ���� ������ �������� �� ������, ���������� ���������� ��� ���������� ���������� � MPICH � 
 *     ����� �������������� ��� �������� ����������� ����� ���������������� �����. ��������� ��������: 
 *     \c ch_gm (Myrinet, ���-1000�), \c ch_p4 (Ethernet), \c ch_shmem (����������� ������, SMP).
 *
 *   - \c -with-mpich-dir=�������, \c --with-gm-dir=�������, \c --with-p4-dir=������� \n
 *     ��� ��������� ������ ������������ ���������, � ������� ����������� ���������� MPICH, 
 *     GM (��� ���� Myrinet) � ��������� MPICH (��� p4, Ethernet). �������� �� ��������� 
 *     �������� ��� ����������� ����������� ���������.
 *   
 *   - \c --with-max-procs=����� \n
 *     ������ ������������ ���������� ������ ������������ ������ (�������� �� ���������: 1024).
 *   
 *   - \c --with-max-mmaped=����� \n
 *     ������ ������������ ���������� ����������� � ������, ������� ���������� ������ ��������� � 
 *     ������������. (�������� �� ���������: 256.)
 *
 *   - \c --with-max-files=����� \n
 *     ������ ������������ ���������� �������� ������������, ������� ������ ��������� ����������. 
 *     (�������� �� ���������: 128).    
 *
 *   - \c --with-max-mem-areas=����� \n
 *     ������ ������������ ���������� ���������� � ������ �������� ��������� ������������ ��������. 
 *     ��� ��������, ������� ������������ �������� ckptIncludeMem/ckptExcludeMem, ����� ������������ 
 *     ���������� ����� ���������. (�������� �� ���������: 8192).
 *
 *   - \c --disable-compression \n
 *     ��������� �� ���������� ���������� ������ ����������� �����.
 *
 *   - \c --disable-benchmarking \n
 *     ��������� �� ���������� ���������� ����� ���������� �� �������������.
 *
 * \subsection linking ����������� ���������� �� � ����������
 *
 * \subsubsection linkc ��� �������� �� ����� �/C++
 *
 * � �������� ���� ��������� ���������� �������� ��� ������� \c main() �� \c ckpt_main(), ��� ������������ 
 * �������� ���������� ������ ��������� � �������� \c MPI_Init() � \c MPI_Finalize(). ��� �������� ���������� �������������, 
 * ���� � ������� ������ ��������� �������� ������������ ���� \c checkpoint.h, ���������������� ������ � �����������. 
 * ����� ���������� � ��������� ������ ����������� ��������� � ������� �������� ����������� ����� \c ckptCheckpoint().
 *
 * ��������� ������ ���� ������� ���������� � �������������� ���������� ����������� ����� � ������������ ����������� 
 * (��� ��������� ���������� � ������������ �������). ������ \c Makefile ��� ������������� ����� ������ ����� ����� � 
 * �������� \c test/mpi/ ������������. 
 *
 * \subsubsection linkfortran ��� �������� �� ����� Fortran
 *
 * � �������� ���� ���������� �������� \c 'PROGRAM \c xxx' �� \c 'SUBROUTINE \c ckpt_main' � ������ ��������� � 
 * \c MPI_Init() � \c MPI_Finalize(). ����� � ������ ������ ����������� ������ \c ckptCheckpoint(). ������ 
 * �������������� � ���� �� �����������, ��� � ��� ��������� �� ����� C, ������ ������� ������������ ������� ���������� ��� 
 * ����� Fortran \c libfcheckpoint.
 *
 * \section interface ��������� ����������
 *
 * ��������� ���������� ���������� ��������� �������:
 *   - ckptCheckpoint()
 *   - ckptIncludeMem()        
 *   - ckptExcludeMem()        
 *   - ckptBenchmarkPrint()        
 *
 * \section tuning ���������
 *
 * \subsection tuningvars ��������� ����������
 *  
 *  ������ ���������� ����������, � ������� - �������� �� ���������.
 *
 *   - \c enable (1) \n
 *      ���������/���������� ���������� �������� ��.
 *      ���� 0, �� ����������� �������� �� � �������������� �� ��. ���������� 
 *      �� �������������.
 *   - \c directory (".")  \n 
 *      �������, � ������� �������� ����������� �����. �� ��������� ��� ��������
 *      � ������� ��������.
 *   - \c alarm_enable (0) \n  
 *      �������� �������������� ������������� �������� ��. <b>������������ � MPI.</b>
 *   - \c auto_period (600) \n   
 *      �������� (� ��������) ����� ��������������� ���������� ��.
 *   - \c hup_enable (1) \n 
 *      ��������� �� �� ������� ������� SIGHUP? <b>������������ � MPI.</b>
 *   - \c async_enable (0) \n 
 *      ����������� (�������) �������� �� ��� ������ fork(). <b>������������ � MPI.</b>
 *   - \c mtime_check_disable (0) \n 
 *      ��������� �������� ��� �������������� �� ������������ ������� 
 *      ��������� ����������� ������������ �����
 *   - \c checkpoints_in_subdirs (0) \n 
 *      ���� 1, �� �� �������� � ������������ � ����������� �� ������ �������� ��.
 *   - \c num_checkpoints_to_keep (1) \n 
 *      ���������� ������� ��, ������� ��������. ���� 1, �� �������� ������ ���������
 *      ��������� ��.
 *              
 * \subsection tuningfiles ����� ���������
 *
 * ���������� �������� ��������� ������� ���� ��������� <tt>%PREFIX%/etc/libcheckpoint.conf</tt>,
 * ����� ���� <tt>.libcheckpoint</tt> � �������� �������� �������� ������������. ��� ����� �����
 * ���������� ������:
 *
 * <tt>��������=��������</tt>
 *
 * ��������,
 *
 * <tt>
 * directory=/var/db/checkpoints \n                                                                                  
 * enable=1 \n                                                                                               
 * checkpoints_in_subdirs=0 \n                                                                               
 * num_checkpoints_to_keep=3                                                                               
 * </tt>
 *
 * \subsection tuningenv ��������� ����� ���������� ���������
 *
 * ����� ������� ��������� ����� ���������, ���������� ����� ���������� ���������. ��� �������� ���������
 * ����� ���������� ��������� ��� ��� ���� ��������� � ������� ������� � �������� ������� \c CKPT_, ��������, ���
 * Bourne-shell:
 *
 * <tt>CKPT_DIRECTORY=/var/db/checkpoints CKPT_ASYNC_ENABLE=1 ./program</tt>
 *
 * \section faq FAQ
 *
 * \subsection faq1 �������� ��� ������ ��������, ��������� � ����������� ��
 *
 *   - <em>������ ��������� ����������� � 'Abort trap'?</em> \n
 *     ������ ��������� ��������������� � ���, ��� ��� ������ ���������� ��������� ��������� ������, 
 *     ������� �� ��������� ���������� ���������� ���������. ������ ����� ���� ������� ���������� ������� 
 *     ����� ���������� ��������� �� ������. ����� ��������� ���������� ����� �������� ��� ������ 
 *     backtrace �� core-����� ���������. 
 *
 * \subsection faq2 ����������� �� ���������� ����������
 *
 *   - <em>��� ����� ����������� �������� �����?</em> \n
 *     ���������� ���������� ��� ��������� ��������� � �������� ��������� ���������� open, close, dup, dup2, 
 *     read, write, readv, writev. ���� �������� ���������� ��� ������� � ����� ���� �������, ���������� 
 *     �� ������ ��������� ��� ��������� (���, �� ����� ��������� ������). ��� �������� ������������ ����������� 
 *     ���������, ������� ���� �������� ������� open, � ����� ��������� ��������� ���������. ��� �������� ������������, 
 *     ���������� ��� ������ ������� dup/dup2 �� ����������� ��������� '������' ��������� ���������.
 *   - <em>��� ����� ����������� ����������� � ������?</em> \n
 *     ����������� � ������ ����������� ���� ��������� ��������� � ��������� �������� mmap � munmap (��� Linux 
 *     ��� � � �������� __mmap � __munmap). ���� ����������� �� ���� ������� � ������ (�������� fd == -1), �� � ����������� 
 *     ����� ����������� � ���������� ������� ������. ���� ����������� ���� ������� � ������, � �������� ���������� ��������� 
 *     ��� ��������� ���������� (��. ���������� ������), �������� ���������� ����� ��������, � ����������� ����� ����������������� 
 *     ����������. ���� �� �������� ����������, ��������� � ������������, �� ��������� ��� ��������� ����������, ����� ��������� 
 *     ���������� ������� ������, � ����� ����������������� ��� ����� � ������.
 *   - <em>���������� �� ���������� malloc(3)?</em> \n
 *     ���, �������� � ���������� ����������� ������ � ������������� ������������� ������. ����� ��������� ����� ����������� 
 *     ��� ������ ������ ������� ����������� ����������, �������� printf(3).
 *   - <em>����� �������� ������������ ��� ������ ������ ����������� �����?</em> \n
 *     XXX
 *
 * \section licence ��������
 *
 * ���������� ������������� �� �������� BSD. ����� �,
 * ���������� minilzo, ���������������� �� �������� GPL, 
 * �� ����� ��������� ��� ���������� ���������� \c --disable-compression.
 *
 * <tt>
 * Copyright (c) 2002-2007 Andrey Smirnov. All rights reserved.
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
 * </tt>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "setup.h"
#include "image.h"
#include "files.h"
#include "elf_.h"
#include "debug.h"
#include "system.h"
#include "vital.h"
#include "restore.h"
#include "areas.h"
#include "benchmark.h"

#ifdef WITH_MPICH
#include "mpich/mpich.h"
#endif

/**
 * ��������� ��������� �� ������� �������� (� ������� �������) 
 *
 * \param p ���������
 */

#define PAGE_ALIGN(p)	((void *)(((size_t)(p) + GPAGESIZE-1)/GPAGESIZE*GPAGESIZE))

/* ����������� ���� ������� */ 
int                     ckptInit(int *argc, char ***argv);
int		        ckptDone();
static int              ckptInitAlarm();
static int              ckptInitSignal();
static int              ckptInitAsync();
static int	        ckptInitMemoryAreas();
int                     ckptCheckRestore();
static RETSIGTYPE       ckptSIGALRM_Handler(int);
static RETSIGTYPE       ckptSIGHUP_Handler(int);
static RETSIGTYPE       ckptSIGCHLD_Handler(int);
int                     ckptCheckpoint();
static int 	        ckptCheckpointHeader(ckptImage * image);
static int	        ckptCheckpointData(ckptImage * image, void *start, size_t size, int kind);
static int	        ckptCheckpointStack(ckptImage * image);

/**
 * XXX: ��� ���? 
 */

static void    *ckptOldBrk;

/**
 * ����� ������� - ����� �������� ������ (UNIX) 
 */

extern int      end;

/* PID of the child used in asynchronous checkpointing */
static pid_t	child_pid;

extern int ckpt_main(int argc, char **argv); 

/* Override program entry point */
int
__ckpt_entry(int argc, char **argv)
{
	int result;
	
	ckptReadSetup(argv);

	if (ckptEnable)
		ckptInit(&argc, &argv);

	result = ckpt_main(argc, argv);
	
	if (ckptEnable)
		ckptDone();
		
	return result;
}

/* Initialise checkpointing engine */
int
ckptInit(int *argc, char ***argv)
{
#ifdef ENABLE_BENCHMARK
	if (!ckptBenchmarkInit())
		abort();
#endif
	
	CKPT_BENCHMARK(CKPT_BNCH_PROG_START);

#ifdef ENABLE_COMPRESSION
	if (!ckptImageInitCompression())
		abort();
#endif

	if (!ckptInitMemoryAreas())
		abort();

#ifdef WITH_MPICH
        /* ���� �������� � ������������ ������, ��������
         * �� ����� ������� � ���������� ��������� 
         */
        ckptAlarmEnable = 0;
        ckptSignalEnable = 0;
        ckptAsyncEnable = 0;
#endif

	/* until we remember signals, we should do this first */
	if (ckptAlarmEnable)
		ckptInitAlarm();
	if (ckptSignalEnable)
		ckptInitSignal();
	if (ckptAsyncEnable)
		ckptInitAsync();

	
#ifdef  WITH_MPICH
	if (!ckptVitalInit())
		abort();

	if (!ckptMpichInit(argc, argv))
		abort();
#endif	

#ifdef 	WITH_MPICH
	if (ckptMpichCheckRestore())
#else
	if (ckptCheckRestore())
#endif
		ckptRestore();

	return 1;
}

/* Finalise checkpointing engine */
int
ckptDone()
{
#ifdef  WITH_MPICH
	ckptMpichDone();
	ckptVitalDone();
#endif	

#ifdef	ENABLE_COMPRESSION
	ckptImageDoneCompression();
#endif

	CKPT_BENCHMARK(CKPT_BNCH_PROG_FINISH);

#ifdef 	ENABLE_BENCHMARK
	ckptBenchmarkDone();
#endif
	
	return 1;
}

/* Initialise automatic checkpointing */
static int
ckptInitAlarm()
{
	signal(SIGALRM, ckptSIGALRM_Handler);
	alarm(ckptAlarmTimeout);
	return 1;
}

/* Initialise SIGHUP checkpointing */
static int
ckptInitSignal()
{
	signal(SIGHUP, ckptSIGHUP_Handler);
	return 1;
}

/* Initialise asynchronous checkpointing */
static int
ckptInitAsync()
{
	signal(SIGCHLD, ckptSIGCHLD_Handler);
	return 1;
}

/* Initialize memory areas, which should be automagically checkpointed */
static int
ckptInitMemoryAreas()
{
	void           *start;
	size_t          size;

	/* data sections & BSS, information via ELF header */
	if (ckptElfOpen(ckptExecutable))
	{
		while (ckptElfGetSection(&start, &size))
			ckptIncludeMemArea(start, size, CHNK_DATA);
		ckptElfClose();
	}
	else
		return 0;
	
	/* heap */
	start = HEAP_START; /* defined in config.h */
	ckptOldBrk = sbrk(0);
	if (ckptOldBrk > start)
		ckptIncludeMemArea(start, (char *)ckptOldBrk - (char *)start, CHNK_HEAP);
		
	return 1;
}

/* Check whether we should restore at this point */
int
ckptCheckRestore()
{
	char           *filename = ckptGetFinalFilename();
	int             result = ckptFileExists(filename);
	return result;
}

/* SIGALRM handler */
static RETSIGTYPE
ckptSIGALRM_Handler(int sig)
{
	ckptCheckpoint();
	signal(SIGALRM, ckptSIGALRM_Handler);
	alarm(ckptAlarmTimeout);
}

/* SIGHUP handler */
static RETSIGTYPE
ckptSIGHUP_Handler(int sig)
{
	ckptCheckpoint();
	signal(SIGHUP, ckptSIGHUP_Handler);
}

/* SIGCHLD handler */
static RETSIGTYPE
ckptSIGCHLD_Handler(int sig)
{
	int dummy;
	
	if (child_pid)
	{
	    if (waitpid(child_pid, &dummy, WNOHANG) == child_pid)
		child_pid = 0;
	}    
	
	signal(SIGCHLD, ckptSIGCHLD_Handler);
}

/**
 * ������� ���������� �������� ����������� �����.
 *
 * ��� �������������� �� �� �� ���������� ���, ��� ���� �� ��������� ������� �� ��. ������ ������� 
 * �������� �������� � ���������� ����������, � ������������� �������� ������������. 
 *
 * \return 
 *    - 0 - �������� �������� ����������� �����
 *    - 1 - ������� ����� ��������������
 *    - -1 - ������ ��� �������� ����������� �����
 */

int ckptCheckpoint()
{
	static int in_checkpointing = 0;
	
	/* engine not enabled */
	if (!ckptEnable)
	    return 0;
	
	/* already checkpointing, don't allow such calls */
	if (in_checkpointing)
	    return 0;
	
	/* previous asynchronous checkpoint hasn't been finished */
	if (ckptAsyncEnable && child_pid)
	    return 0;
	    
	/* handle asynchronous checkpointing and remember child pid */    
	if (ckptAsyncEnable)
	{  
	    child_pid = fork();
	    if (child_pid)
		return 0;
	}
	
	in_checkpointing = 1;

	CKPT_BENCHMARK(CKPT_BNCH_CKPT_BEGIN);
	
	/* remember signals and CPU registers */
	if (!sigsetjmp(ckptJumpBuffer,0xff))
	{
		char                   *temp_filename = ckptGetTempFilename();
		ckptImage       	image;
		struct ckptMemArea     *t;
		void		       *NewBrk;

#ifdef WITH_MPICH
		if (!ckptMpichBeforeCheckpoint())
                        return -1;
#endif
		ckptCreateImage(&image, temp_filename);

		ckptCheckpointHeader(&image);

		NewBrk = sbrk(0);
		if (NewBrk > ckptOldBrk)
		{
			ckptIncludeMemArea(ckptOldBrk, (char *)NewBrk - (char *)ckptOldBrk, CHNK_HEAP);
			ckptOldBrk = NewBrk;
		}
		else if (NewBrk < ckptOldBrk)
		{
			ckptExcludeMemArea(NewBrk, (char *)ckptOldBrk - (char *)NewBrk);
			ckptOldBrk = NewBrk;
		}

		/* memory areas */
		/* at first, CHNK_DATA */
		for (t = ckptAreaHead; t != NULL; t = t->next)
			if (t->kind == CHNK_DATA)
				ckptCheckpointData(&image, t->start, t->size, t->kind);
		/* then, all other */
		for (t = ckptAreaHead; t != NULL; t = t->next)
			if (t->kind != CHNK_DATA && t->kind != CHNK_EXCLUDE)
				ckptCheckpointData(&image, t->start, t->size, t->kind);

		/* stack */
		ckptCheckpointStack(&image);

		ckptCloseImage(&image);

		ckptRenameToFinal(temp_filename);

#ifdef WITH_MPICH		
		ckptMpichAfterCheckpoint();
#endif

		in_checkpointing = 0;

		CKPT_BENCHMARK(CKPT_BNCH_CKPT_END);
		
		if (ckptAsyncEnable)
		    _exit(0);
		
		return 0;
	}

	ckptRestoreFiles();
	ckptRestoreMmap(1);

	printf("<<< Restore finished!\n");

	CKPT_BENCHMARK(CKPT_BNCH_REST_END);
	in_checkpointing = 0;
	
	return 1;
}

/* Write checkpoint header */
static int
ckptCheckpointHeader(ckptImage * image)
{
	struct ckptCheckpointHdr header;

	header.brk = (void *) sbrk(0);
	header.stack_start = (void *)STACK_TOP;
	header.np = CKPT_NP;
	header.mtime = ckptElfGetMtime();

	return ckptWriteChunk(image, CHNK_HEADER, (size_t)sizeof(header), (void *)NULL, (void *)&header);
}

/* Write memory segment */
static int
ckptCheckpointData(ckptImage * image, void *start, size_t size, int kind)
{
	if (size == 0)
		return 1;

	return ckptWriteChunk(image, kind, size, start, start);
}

/* Write stack */
static int
ckptCheckpointStack(ckptImage * image)
{
	int             dummy;

	void           *start = (void *)STACK_TOP;
	size_t     	size = (size_t) ((char *)start - (char *)&dummy);

	return ckptWriteChunk(image, CHNK_STACK, size - STACK_GAP, (void *)NULL, (void *)((char *)start - size));
}


