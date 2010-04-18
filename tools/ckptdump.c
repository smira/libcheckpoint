/*-
 * Copyright (c) 2002-2005 Andrey Smirnov.
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
 * Dumps checkpoint images created by 
 * this library
 *
 */

#include <stdio.h>

#include "../src/image.h"
#include "config.h"

/* hack */
int ckptMpichNumber()
{
	return 0;
}

void usage(char *argv[])
{
	printf("ckptdump " PACKAGE_VERSION "\n");
	printf("usage: %s checkpoint_image\n", argv[0]);
}

int main(int argc, char *argv[])
{
	ckptImage image;
	int type;
	size_t size;
	void *addr;
	
	if (argc != 2)
	{
		usage(argv);
		return 0;
	}

	if (ckptOpenImage(&image, argv[1]) == NULL)
		return 1;

	while (ckptReadChunk(&image, &type, &size, &addr))
	{
		char *stype;

		switch(type)
		{
			case CHNK_HEADER:
				stype = "Header";
				break;
			case CHNK_STACK:
				stype = "Stack";
				break;
			case CHNK_DATA:
				stype = "Data";
				break;
			case CHNK_MPICH:
				stype = "MPI";
				break;
			case CHNK_INTERNAL:
				stype = "Internal";
				break;
			case CHNK_HEAP:
				stype = "Heap";
				break;
			case CHNK_MMAP:
				stype = "Mmap";
				break;
			default:
				stype = "Unknown";
		}

		printf("Chunk [%-10s], addr [%16p], size [%08zx], compr. size [%08zx], ratio [%.4f]\n", 
				stype, addr, size, image.next, (float)image.next/size);
		if (type == CHNK_HEADER)
		{
			struct ckptCheckpointHdr header;
			ckptReadChunkData(&image, &header);

			printf("\tEnd of heap:             %p\n", header.brk);
			printf("\tStack start:             %p\n", header.stack_start);
			printf("\tNumber of processes:     %d\n", header.np);
			printf("\tMtime of image file:     %lu\n", header.mtime);
			
		}
		else
			ckptReadChunkSkip(&image);
	}

	ckptCloseImage(&image);

	return 0;
}

