/*-
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
 * \file image.h
 *
 * ������ � ������� ����������� ����� � �����
 */

#ifndef __CKPT_IMAGE_H__
#define __CKPT_IMAGE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/**
 * ���������� ��������� ��� ������ � ������� �� 
 */

typedef struct _ckptImage
{
	int             fd;                     /**< �������� ���������� �����, ��� �������� ����� */
	size_t          next;                   /**< ������ ���������� ����� */
	int		is_compressed;          /**< ���� �� ��������� ����? */
}               ckptImage;

/**
 * ������� ����� ����� ����������� �����
 */

ckptImage      *ckptCreateImage(ckptImage * image, char *fname);

/**
 * ������� ������������ ���� � ������� ��
 */

ckptImage      *ckptOpenImage(ckptImage * image, char *fname);

/**
 * ������� ����� ��, ��������� ��� ������ ckptOpenImage ��� ckptCreateImage
 */

int             ckptCloseImage(ckptImage * image);

/**
 * �������� � ����� �� ������ �� ������
 */

int             ckptWriteChunk(ckptImage * image, int type, size_t size, void *addr, void *data);

/**
 * ��������� ������ � ��� ���������� ����� � ����� � ��
 */

int             ckptReadChunk(ckptImage * image, int *type, size_t * size, void **addr);

/**
 * ���������� ������ ����� � ������ ��
 */

int             ckptReadChunkSkip(ckptImage * image);

/**
 * ��������� ������ ����� �� ������ �� � ������
 */

int             ckptReadChunkData(ckptImage * image, void *data);

/**
 * ������ ������ ������ �� ����� � ������ 
 */

int 		ckptRewindImage(ckptImage * image);

#ifdef ENABLE_COMPRESSION

/**
 * ������������� ���������� ������ �� 
 */

int 		ckptImageInitCompression();

/**
 * ���������� ���������� ������ �� 
 */

void 		ckptImageDoneCompression();

#endif

/* ���� ������ */

/** ��� �����: ��������� ������ �� */
#define CHNK_HEADER		0x01
/** ��� �����: ���� */
#define CHNK_STACK		0x02
/** ��� �����: ������� ������ */
#define	CHNK_DATA		0x03
/** ��� �����: ����������� ������ MPICH */
#define	CHNK_MPICH		0x04
/** ��� �����: ���������� ������ ���������� */
#define	CHNK_INTERNAL		0x05
/** ��� �����: ���� */
#define CHNK_HEAP		0x06
/** ��� �����: ������������ ������ (mmap) */
#define CHNK_MMAP		0x07
/** ��� �����: �� ���������� � ����� */
#define CHNK_EXCLUDE		0x08

/**
 * ��������� �� 
 */

struct ckptCheckpointHdr
{
	void      *brk;                 /**< ����� ���� (sbrk) */
	void      *stack_start;         /**< ����� ������ ����� */
	int	   np;                  /**< ���������� ��������� � ������������ ��������� */
	time_t	   mtime;               /**< ���� ���������� ��������� ������������ ����� ��������� */
};

#endif

