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
 *  library, common for all devices
 *
 */

#ifndef __CKPTMPICH_H__
#define __CKPTMPICH_H__

/* Initialise MPICH, before actual checkpoiting begins */
int ckptMpichInit(int *argc, char ***argv);

/* Finalise MPICH */
int ckptMpichDone();

/* Get node number */
int ckptMpichNumber();

/* Get total number of processes */
int ckptMpichNP();

/* Checks whether we should restore now */
int ckptMpichCheckRestore();

/* Hooks from main code to support checkpointing with MPICH */
int ckptMpichBeforeCheckpoint();
void ckptMpichAfterCheckpoint();
void ckptMpichBeforeRestore();
void ckptMpichInsideRestore();
void ckptMpichAfterRestore();
void ckptMpichRestoreConvert(void **start);

#endif /* __CKPTMPICH_H__ */

