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
 * \file image.c
 *
 * Работа с образом контрольной точки в файле
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "image.h"
#include "debug.h"
#include "system.h"

#ifdef ENABLE_COMPRESSION
#include "minilzo/minilzo.h"
#endif

/**
 * Заголовок блока в файле КТ 
 */

typedef struct _chunk_header
{
	int             type;                           /**< тип блока */
	void           *addr;                           /**< адрес в памяти, по которому он должен располагаться */
	size_t          size;                           /**< размер блока в памяти */
	int		is_compressed;                  /**< использовалось ли сжатие при записи блока? */
	size_t		compressed_size;                /**< размер в сжатом состоянии */
}               chunk_header;


#ifdef ENABLE_COMPRESSION

/**
 * Заголовок блока сжатых данных (внутри блока данных, если использовалось сжатие) 
 */

typedef struct _compressed_header
{
	size_t		size;                           /**< размер до сжатия */
	lzo_uint	compressed_size;                /**< сжатый размер */
} compressed_header;

/**
 * Буфер для сжатия 
 */

static void *buffer;

/**
 * Буфер для сжатия 
 */

static void *compress_work_buffer;

/**
 * Буфер для сжатия 
 */

static void *compress_temp;

/**
 * Размер блока сжатия 
 */

#define COMPRESS_CHUNK		(1024*1024L)

/**
 * Параметр LZO1_X 
 */

#define COMPRESS_WORK_SIZE	(COMPRESS_CHUNK + COMPRESS_CHUNK / 64 + 16 + 3)

/**
 * Параметр LZO1_X 
 */

#define BUFFER_SIZE 		(LZO1X_1_MEM_COMPRESS + COMPRESS_WORK_SIZE)

/**
 * Минимальный размер блока, для которого включается сжатие 
 */

#define MIN_COMPRESS_CHUNK	(1024)

/**
 * Минимум 
 */

#define	MIN(a,b)		((a) > (b) ? (b) : (a))

#endif

/**
 * Создать новый образ контрольной точки
 *
 * \param result указатель на структу, описывающую образ КТ
 * \param fname имя файла, в котором создается КТ
 * \return указатель, который используется при дальнейших обращениях к функциям этого модуля, или NULL в случае ошибки
 */

ckptImage *ckptCreateImage(ckptImage * result, char *fname)
{
	result->fd = F_REAL_OPEN(fname, O_CREAT | O_WRONLY, 0644);
	if (result->fd == -1)
	{
		ckptLog(CKPT_LOG_ERROR, "Unable to create file %s: %m", fname);
		return NULL;
	}
	return result;
}

/**
 * Открыть существующий файл с образом КТ
 *
 * \param result указатель на структу, описывающую образ КТ
 * \param fname имя файла, в котором находится КТ
 * \return указатель, который используется при дальнейших обращениях к функциям этого модуля, или NULL в случае ошибки
 */

ckptImage *ckptOpenImage(ckptImage * result, char *fname)
{
	result->fd = F_REAL_OPEN(fname, O_RDONLY, 0);
	if (result->fd == -1)
	{
		ckptLog(CKPT_LOG_ERROR, "Unable to open file %s: %m", fname);
		return NULL;
	}
	return result;
}

/**
 * Закрыть образ КТ, созданный при помощи ckptOpenImage или ckptCreateImage
 *
 * \param image указатель на структу, описывающую образ КТ
 * \return 1, если всё успешно
 */

int ckptCloseImage(ckptImage * image)
{
	F_REAL_CLOSE(image->fd);
	return 1;
}

/**
 * Начать чтение образа КТ снова с начала 
 *
 * \param image указатель на структу, описывающую образ КТ
 * \return 1, если всё успешно
 */

int ckptRewindImage(ckptImage * image)
{
	F_REAL_LSEEK(image->fd, 0, SEEK_SET);
        return 1;
}

/**
 * Записать в образ КТ данные из памяти
 *
 * \param image указатель на структу, описывающую образ КТ
 * \param type тип блока
 * \param size размер блока
 * \param addr адрес, который будет подсказкой при восстановлении (куда восстанавливать)
 * \param data адрес блока в памяти
 * \return 1, если всё успешно
 */

int ckptWriteChunk(ckptImage * image, int type, size_t size, void *addr, void *data)
{
	static chunk_header header;
#ifdef 	ENABLE_COMPRESSION
	static compressed_header c_header;
	size_t processed = 0;
	off_t header_position;
#endif


	header.type = type;
	header.size = size;
	header.addr = addr;

#ifdef ENABLE_COMPRESSION
	header.is_compressed = (size > MIN_COMPRESS_CHUNK) && (type != CHNK_STACK);
	if (header.is_compressed)
		header.compressed_size = 0;
	else
		header.compressed_size = size;
	header_position = F_REAL_LSEEK(image->fd, 0, SEEK_CUR);
#else
	header.is_compressed = 0;
	header.compressed_size = size;
#endif

	if (F_REAL_WRITE(image->fd, &header, sizeof(header)) != sizeof(header))
	{
		ckptLog(CKPT_LOG_ERROR, "Unable to write chunk header");
		return 0;
	}

#ifdef 	ENABLE_COMPRESSION

	if (header.is_compressed)
	{
		while (processed < size)
		{
			c_header.size = MIN(size - processed, COMPRESS_CHUNK);
		
			if (lzo1x_1_compress((const lzo_byte *)addr + processed, c_header.size, compress_temp, 
						&c_header.compressed_size, compress_work_buffer) != LZO_E_OK)
			{
				ckptLog(CKPT_LOG_ERROR, "Error while compressing");
				return 0;
			}

			processed += c_header.size;
			header.compressed_size += c_header.compressed_size + sizeof(c_header);

			if (F_REAL_WRITE(image->fd, &c_header, sizeof(c_header)) != sizeof(c_header))
			{
				ckptLog(CKPT_LOG_ERROR, "Error while writing compressed header");
				return 0;
			}

			if (F_REAL_WRITE(image->fd, compress_temp, c_header.compressed_size) != c_header.compressed_size)
			{
				ckptLog(CKPT_LOG_ERROR, "Error while writing compressed data");
				return 0;
			}
		}

		F_REAL_LSEEK(image->fd, header_position, SEEK_SET);
		if (F_REAL_WRITE(image->fd, &header, sizeof(header)) != sizeof(header))
		{
			ckptLog(CKPT_LOG_ERROR, "Error while rewriting header");
			return 0;
		}
		F_REAL_LSEEK(image->fd, header.compressed_size, SEEK_CUR);
	}
	else
	{
		if (F_REAL_WRITE(image->fd, data, size) != size)
		{
			ckptLog(CKPT_LOG_ERROR, "Error while writing data, offset %p, size %x: %m",
				data, size);
			return 0;
		}
	}

#else
	if (F_REAL_WRITE(image->fd, data, size) != size)
	{
		ckptLog(CKPT_LOG_ERROR, "Error while writing data, offset %p, size %x: %m",
			data, size);
		return 0;
	}
#endif
	return 1;
}

/**
 * Прочитать размер и тип следующего блока в файле с КТ
 *
 * \param image указатель на структу, описывающую образ КТ
 * \param type сюда будет записан тип следующего блока
 * \param size сюда будет записан размер следующего блока
 * \param addr сюда будет записан адрес-подсказка места востановления данных
 * \return 1, если всё успешно или 0 в случае ошибки (или в конце файла)
 */

int ckptReadChunk(ckptImage * image, int *type, size_t * size, void **addr)
{
	static chunk_header header;

	int             result = F_REAL_READ(image->fd, &header, sizeof(header));

	if (result == -1)
	{
		ckptLog(CKPT_LOG_ERROR, "Unable to read chunk header");
		return 0;
	}
	if (result == 0)
		return 0;

#ifndef ENABLE_COMPRESSION
	if (header.is_compressed)
	{
		ckptLog(CKPT_LOG_ERROR, "Attempt to read compressed chunk with version without compression");
		return 0;
	}
#endif
	*type = header.type;
	*size = header.size;
	*addr = header.addr;
	image->next = header.compressed_size;
	image->is_compressed = header.is_compressed;

	return 1;
}

/**
 * Пропустить данные блока в образе КТ
 *
 * \param image указатель на структу, описывающую образ КТ
 * \return 1, если всё успешно
 */

int ckptReadChunkSkip(ckptImage * image)
{
	return F_REAL_LSEEK(image->fd, image->next, SEEK_CUR) != -1;
}

/**
 * Прочитать данные блока из образа КТ в память
 *
 * \param image указатель на структу, описывающую образ КТ
 * \param data адрес восстановления данных
 * \return 1, если всё успешно
 */

int ckptReadChunkData(ckptImage * image, void *data)
{
#ifdef ENABLE_COMPRESSION
	size_t processed = 0;
	lzo_uint decompressed_len;
	compressed_header c_header;

	if (image->is_compressed)
	{
		while (processed < image->next)
		{
			if (F_REAL_READ(image->fd, &c_header, sizeof(c_header)) != sizeof(c_header))
			{
				ckptLog(CKPT_LOG_ERROR, "Error reading compression header");
				return 0;
			}

			if (F_REAL_READ(image->fd, compress_temp, c_header.compressed_size) != c_header.compressed_size)
			{
				ckptLog(CKPT_LOG_ERROR, "Error reading compressed data");
				return 0;
			}

			if (lzo1x_decompress(compress_temp, c_header.compressed_size, data, &decompressed_len, NULL) != LZO_E_OK ||
					decompressed_len != c_header.size)
			{
				ckptLog(CKPT_LOG_ERROR, "Error while decompressing");
				return 0;
			}

			processed += c_header.compressed_size + sizeof(c_header);
			data = (char *)data + c_header.size;
		}
	}
	else
	{
		if (F_REAL_READ(image->fd, data, image->next) != image->next)
		{
			ckptLog(CKPT_LOG_ERROR, "Error reading chunk, offset %p, size %x: %m",
				  data, image->next);
			return 0;
		}
	}
#else
	if (F_REAL_READ(image->fd, data, image->next) != image->next)
	{
		ckptLog(CKPT_LOG_ERROR, "Error reading chunk, offset %p, size %x: %m",
			  data, image->next);
		return 0;
	}
#endif

	return 1;
}

#ifdef ENABLE_COMPRESSION

/**
 * Инициализация подсистемы сжатия КТ 
 *
 * \return 1, если всё успешно
 */

int ckptImageInitCompression()
{
	buffer = F_REAL_MMAP(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (buffer != MAP_FAILED)
	{
		compress_work_buffer = buffer;
		compress_temp = (char *)buffer + LZO1X_1_MEM_COMPRESS;
	}
        else
                return 0;

	if (lzo_init() != LZO_E_OK)
		return 0;

	return 1;
}

/**
 * Остановить подсистему сжатия КТ 
 */

void ckptImageDoneCompression()
{
	F_REAL_MUNMAP(buffer, BUFFER_SIZE);
}

#endif

