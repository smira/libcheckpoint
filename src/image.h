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
 * Работа с образом контрольной точки в файле
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
 * Прозрачная структура для работы с образом КТ 
 */

typedef struct _ckptImage
{
	int             fd;                     /**< файловый дескриптор файла, где хранится образ */
	size_t          next;                   /**< размер следующего блока */
	int		is_compressed;          /**< сжат ли следующий блок? */
}               ckptImage;

/**
 * Создать новый образ контрольной точки
 */

ckptImage      *ckptCreateImage(ckptImage * image, char *fname);

/**
 * Открыть существующий файл с образом КТ
 */

ckptImage      *ckptOpenImage(ckptImage * image, char *fname);

/**
 * Закрыть образ КТ, созданный при помощи ckptOpenImage или ckptCreateImage
 */

int             ckptCloseImage(ckptImage * image);

/**
 * Записать в образ КТ данные из памяти
 */

int             ckptWriteChunk(ckptImage * image, int type, size_t size, void *addr, void *data);

/**
 * Прочитать размер и тип следующего блока в файле с КТ
 */

int             ckptReadChunk(ckptImage * image, int *type, size_t * size, void **addr);

/**
 * Пропустить данные блока в образе КТ
 */

int             ckptReadChunkSkip(ckptImage * image);

/**
 * Прочитать данные блока из образа КТ в память
 */

int             ckptReadChunkData(ckptImage * image, void *data);

/**
 * Начать чтение образа КТ снова с начала 
 */

int 		ckptRewindImage(ckptImage * image);

#ifdef ENABLE_COMPRESSION

/**
 * Инициализация подсистемы сжатия КТ 
 */

int 		ckptImageInitCompression();

/**
 * Остановить подсистему сжатия КТ 
 */

void 		ckptImageDoneCompression();

#endif

/* Типы блоков */

/** Тип блока: заголовок образа КТ */
#define CHNK_HEADER		0x01
/** Тип блока: стек */
#define CHNK_STACK		0x02
/** Тип блока: сегмент данных */
#define	CHNK_DATA		0x03
/** Тип блока: разделяемая память MPICH */
#define	CHNK_MPICH		0x04
/** Тип блока: внутренние данные библиотеки */
#define	CHNK_INTERNAL		0x05
/** Тип блока: куча */
#define CHNK_HEAP		0x06
/** Тип блока: отображенная память (mmap) */
#define CHNK_MMAP		0x07
/** Тип блока: не записывать в образ */
#define CHNK_EXCLUDE		0x08

/**
 * Заголовок КТ 
 */

struct ckptCheckpointHdr
{
	void      *brk;                 /**< конец кучи (sbrk) */
	void      *stack_start;         /**< адрес начала стека */
	int	   np;                  /**< количество процессов в параллельной программе */
	time_t	   mtime;               /**< дата последнего изменения исполняемого файла программы */
};

#endif

