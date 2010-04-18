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

#include "config.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include "vital.h"
#include "system.h"
#include "debug.h"

static char *vital_start;

/* Initialize vital area */
int 
ckptVitalInit()
{
    vital_start = (char *)F_REAL_MMAP(NULL, VITAL_AREA_SIZE, PROT_READ | PROT_WRITE,
                                      MAP_ANON | MAP_SHARED, -1, 0);
    return vital_start != MAP_FAILED;			       
}

/* Release vital area */
void ckptVitalDone()
{
    F_REAL_MUNMAP(vital_start, VITAL_AREA_SIZE);
}

/* Put data into vital area */
void ckptVitalPut(int offset, void *data, int size)
{
    if (offset + size > VITAL_AREA_SIZE)
    {
	    ckptLog(CKPT_LOG_ERROR, "Out of vital area");
	    abort();
    }
    memcpy(vital_start+offset, data, size);
}

/* Get data from vital area */
void ckptVitalGet(int offset, void *data, int size)
{
    if (offset + size > VITAL_AREA_SIZE)
    {
	    ckptLog(CKPT_LOG_ERROR, "Out of vital area");
	    abort();
    }
    memcpy(data, vital_start+offset, size);
}

/* Save vital data while restoring data segment */
char **ckptVitalSave()
{
    return &vital_start;
}

/* 
 * Vital data as type&size-safe stack
 */

static int vital_stack;

struct vital_header
{
	int type;
	int size;
};

/* Init vital stack */
void ckptVitalStackInit()
{
	vital_stack = 0;
}

/* Push vital data: type, size and pointer are passed */
void ckptVitalPush(int type, int size, void *data)
{
	struct vital_header header;
	
	header.type = type;
	header.size = size;
	ckptVitalPut(vital_stack, &header, sizeof(header));
	vital_stack += sizeof(header);

	ckptVitalPut(vital_stack, data, size);
	vital_stack += size;
}

/* Pop vital data */
void ckptVitalPop(int type, int size, void *data)
{
	struct vital_header header;

	ckptVitalGet(vital_stack, &header, sizeof(header));
	if (header.type != type || header.size != size)
	{
		ckptLog(CKPT_LOG_ERROR, "VitalPop mismatch: wanted type [%x], size [%d], got type [%x], size[%d]",
				type, size, header.type, header.size);
		abort();
	}
	vital_stack += sizeof(header);

	ckptVitalGet(vital_stack, data, size);
	vital_stack += size;
}

/* Peek next chunk size & type */
int ckptVitalPeek(int *type)
{
	struct vital_header header;

	ckptVitalGet(vital_stack, &header, sizeof(header));
	if (type != NULL)
		*type = header.type;

	return header.size;
}

