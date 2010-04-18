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
 */

/**
 * \file files.h
 *
 * Генерация и обслуживание имён файлов контрольных точек
 */

#ifndef __FILES_H__
#define __FILES_H__

#ifdef WITH_MPICH

#include "mpich/mpich.h"

/**
 * Номер ветви параллельной программы 
 */

#define CKPT_NUMBER	ckptMpichNumber()

/**
 * Количество ветвей параллельной программы 
 */

#define	CKPT_NP		ckptMpichNP()

#else

#define CKPT_NUMBER	0
#define	CKPT_NP		1

#endif

/**
 * Получить имя временного файла, куда будет записываться образ КТ во 
 * время её создания
 */

char *ckptGetTempFilename();

/**
 * Переименовать временный файл с КТ в окончательный
 */

void ckptRenameToFinal(char *temp_filename);

/**
 * Получить окончательное имя файла для контрольной точки
 */

char *ckptGetFinalFilename();

/**
 * Проверить, существует ли файл
 */

int  ckptFileExists(char *filename);

#endif /* __FILES_H__ */

