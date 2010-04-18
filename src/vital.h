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
 *  Provides a place to store vital data,
 *  which should live after restore
 *
 */

#ifndef __VITAL_H__
#define __VITAL_H__

#define VITAL_AREA_SIZE		1048576

/* Initialize vital area */
int ckptVitalInit();

/* Release vital area */
void ckptVitalDone();

/* Put data into vital area */
void ckptVitalPut(int offset, void *data, int size);

/* Get data from vital area */
void ckptVitalGet(int offset, void *data, int size);

/* Save vital data while restoring data segment */
char **ckptVitalSave();

/* Init vital stack */
void ckptVitalStackInit();

/* Push vital data: type, size and pointer are passed */
void ckptVitalPush(int type, int size, void *data);

/* Pop vital data */
void ckptVitalPop(int type, int size, void *data);

/* Peek next chunk size & type */
int ckptVitalPeek(int *type);


#endif /* __VITAL_H__ */

