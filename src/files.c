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
 * \file files.c
 *
 * Генерация и обслуживание имён файлов контрольных точек
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "setup.h"
#include "files.h"
#include "debug.h"

/**
 * Получить имя временного файла, куда будет записываться образ КТ во 
 * время её создания
 *
 * \return имя временного файла
 */

char *ckptGetTempFilename()
{
	static char     buf[1024];

	if (ckptCheckpointsInSubdirs)
	{
		
		snprintf(buf, sizeof(buf), "%s/%s.0.temp", ckptDirectory, ckptBasename);
		mkdir(buf, 0755); /* We don't check for errors here, because directory may exist */
		snprintf(buf, sizeof(buf), "%s/%s.0.temp/%s.ckpt.%d", ckptDirectory, ckptBasename, ckptBasename, CKPT_NUMBER);
	}
	else
		snprintf(buf, sizeof(buf), "%s/%s.ckpt.0.%d.temp", ckptDirectory, ckptBasename, CKPT_NUMBER);

	return buf;
}

/**
 * Переименовать временный файл с КТ в окончательный
 *
 * \param temp_filename имя временного файла КТ
 */

void ckptRenameToFinal(char *temp_filename)
{
	static char     buf[1024];
	static char     buf2[1024];
        int i;

	if (ckptCheckpointsInSubdirs)
	{
                if (CKPT_NUMBER == 0)
                {

                        for (i = 0; i < CKPT_NP; i++)
                        {
                                snprintf(buf, sizeof(buf), "%s/%s.%d/%s.ckpt.%d", ckptDirectory, ckptBasename, ckptNumCheckpointsKeep-1, 
                                                ckptBasename, i);
                                unlink(buf);
                        }
                        for (i = ckptNumCheckpointsKeep - 1; i > 0; i--)
                        {
                                snprintf(buf, sizeof(buf), "%s/%s.%d", ckptDirectory, ckptBasename, i-1);
                                snprintf(buf2, sizeof(buf), "%s/%s.%d", ckptDirectory, ckptBasename, i);
                                rename(buf, buf2);
                        }
                        snprintf(buf, sizeof(buf), "%s/%s.0.temp", ckptDirectory, ckptBasename);
                        snprintf(buf2, sizeof(buf), "%s/%s.0", ckptDirectory, ckptBasename);
                        rename(buf, buf2);
                }
	}
	else
	{
                if (CKPT_NUMBER == 0)
                {
                        snprintf(buf, sizeof(buf), "%s/%s.ckpt.%d.%d", ckptDirectory, ckptBasename, ckptNumCheckpointsKeep-1, CKPT_NUMBER);
                        unlink(buf);

                        for (i = ckptNumCheckpointsKeep - 1; i > 0; i--)
                        {
                                snprintf(buf, sizeof(buf), "%s/%s.ckpt.%d.%d", ckptDirectory, ckptBasename, i-1, CKPT_NUMBER);
                                snprintf(buf2, sizeof(buf), "%s/%s.ckpt.%d.%d", ckptDirectory, ckptBasename, i, CKPT_NUMBER);
                                rename(buf, buf2);
                        }
                }
		snprintf(buf, sizeof(buf), "%s/%s.ckpt.0.%d", ckptDirectory, ckptBasename, CKPT_NUMBER);
		rename(temp_filename, buf);
	}
}

/**
 * Получить окончательное имя файла для контрольной точки
 *
 * \return окончательное имя файла
 */

char *ckptGetFinalFilename()
{
	static char     buf[1024];

	if (ckptCheckpointsInSubdirs)
		snprintf(buf, sizeof(buf), "%s/%s.0/%s.ckpt.%d", ckptDirectory, ckptBasename, ckptBasename, CKPT_NUMBER);
	else
		snprintf(buf, sizeof(buf), "%s/%s.ckpt.0.%d", ckptDirectory, ckptBasename, CKPT_NUMBER);
	
	return buf;
}

/**
 * Проверить, существует ли файл
 *
 * \param filename имя файла
 * \return истина, если файл существует
 */

int ckptFileExists(char *filename)
{
	struct stat     sb;

	return stat(filename, &sb) == 0;
}
