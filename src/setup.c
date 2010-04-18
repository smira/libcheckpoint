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
 * \file setup.c
 *
 * Чтение и хранение настроек библиотеки
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

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "setup.h"
#include "system.h"

/* Переменные настроки библиотеки */

/**
 * Включение/выключение подсистемы создания КТ 
 */

int             ckptEnable = 1;

/**
 * Каталог хранения контрольных точек 
 */

char            ckptDirectory[1024];

/**
 * Автоматическое периодическое создание КТ 
 */

int             ckptAlarmEnable = 600;

/**
 * Период автоматического создания КТ 
 */

int             ckptAlarmTimeout = 10;

/**
 * Создавать КТ по приходу сигнала SIGHUP? 
 */

int             ckptSignalEnable = 1;

/**
 * Асинхронное создание КТ 
 */

int 		ckptAsyncEnable = 
#ifndef WITH_MPICH
				    0
#else
				    0
#endif				    				    
				     ;
/**
 * Запретить проверку при восстановлении на неизменность времени 
 * последней модификации исполняемого файла
 */

int 		ckptDisableMtimeCheck = 0;

/**
 * Хранить КТ в подкаталогах? 
 */

int		ckptCheckpointsInSubdirs = 0;

/**
 * Количество хранимых КТ 
 */

int		ckptNumCheckpointsKeep = 1;

/* Другие переменные */

/**
 * Полный путь к исполняемому файлу 
 */

char            ckptExecutable[1024];

/**
 * Имя исполняемого файла 
 */

char            ckptBasename[1024];

/**
 * Буфер sigjmp, используется при восстановлении из КТ 
 */

sigjmp_buf      ckptJumpBuffer;

/**
 * Тип перменной - целое 
 */

#define VAR_INT			0

/**
 * Тип переменной - строка 
 */

#define VAR_STR			1

/**
 * Описание переменных настройки библиотеки 
 */

typedef struct
{
	int             kind;                   /**< тип переменной (целое/строка) */
	char           *name;                   /**< имя переменной */
	char           *svalue;                 /**< строковое значение (VAR_STR) */
	int            *ivalue;                 /**< целое значение (VAR_INT) */
}               ckptSetupVariable;

/**
 * Переменные настройки библиотеки 
 */

ckptSetupVariable ckptSetupVariables[] =
{
	{VAR_INT, "enable", NULL, &ckptEnable},
	{VAR_STR, "directory", ckptDirectory, NULL},
	{VAR_INT, "alarm_enable", NULL, &ckptAlarmEnable},
	{VAR_INT, "auto_period", NULL, &ckptAlarmTimeout},
	{VAR_INT, "hup_enable", NULL, &ckptSignalEnable},
	{VAR_INT, "async_enable", NULL, &ckptAsyncEnable},
	{VAR_INT, "mtime_check_disable", NULL, &ckptDisableMtimeCheck},
	{VAR_INT, "checkpoints_in_subdirs", NULL, &ckptCheckpointsInSubdirs},
	{VAR_INT, "num_checkpoints_to_keep", NULL, &ckptNumCheckpointsKeep},
	{-1},
};

/**
 * Минимум :) 
 */

#define MIN(a,b)	((a) < (b) ? (a) : (b))

/**
 * Попытаться прочитать настройки из файла
 *
 * \param filename имя файла с настройками
 * \return 1, если успешно, 0 иначе
 */

static int ckptReadSetupFile(char *filename)
{
	int 		fd;
	char   	       *buf, *p, *p2;
	struct stat     sb;
	char            line[1024];
	off_t		length;

	fd = F_REAL_OPEN(filename, O_RDONLY, 0);
	if (fd == -1)
		return 0;

	if (fstat(fd, &sb) == -1)
	{
		F_REAL_CLOSE(fd);
		return 0;
	}

	buf = F_REAL_MMAP(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED)
	{
		F_REAL_CLOSE(fd);
		return 0;
	}

	F_REAL_CLOSE(fd);

	p = buf;
	length = sb.st_size;
	while (length > 0)
	{
		int len;
		char *value;
		ckptSetupVariable *var;

		memset(line, '\0', sizeof(line));
		p2 = memchr(p, '\n', length);
		if (p2 == NULL)
		{
			memcpy(line, p, MIN(length, sizeof(line)-1));
			length = 0;
		}
		else
		{
			memcpy(line, p, MIN(p2-p, sizeof(line)-1));
			length -= p2-p+1;
			p = p2 + 1;
		}

		len = strlen(line);

		if (len > 0 && line[len-1] == '\n')
			line[--len] = '\0';

		value = strchr(line, '=');
		if (value == NULL)
			continue;

		*value++ = '\0';
		
		for (var = ckptSetupVariables; var->kind >= 0; var++)
			if (strcasecmp(var->name, line) == 0)
			{
				if (var->kind == VAR_STR)
					strcpy(var->svalue, value);
				else if (var->kind == VAR_INT)
					*var->ivalue = atoi(value);
			}
	}
	
	F_REAL_MUNMAP(buf, sb.st_size);

	return 1;
}

/**
 * Прочитать настройки из переменных окружения вида CKPT_XXXX 
 */

static void ckptReadSetupEnv()
{
	ckptSetupVariable *var;
	char            buf[128];

	for (var = ckptSetupVariables; var->kind >= 0; var++)
	{
		char           *p;

		strcpy(buf, "CKPT_");
		strcat(buf, var->name);

		for (p = buf; *p; p++)
			*p = toupper(*p);

		p = getenv(buf);
		if (p)
		{
			if (var->kind == VAR_STR)
				strcpy(var->svalue, p);
			else if (var->kind == VAR_INT)
				*var->ivalue = atoi(p);
		}
	}
}

/**
 * Установить значения по умолчанию, если они не были установлены при чтении настроек 
 */

static void ckptSetupDefaults()
{
	if (*ckptDirectory == '\0')
		strcpy(ckptDirectory, ".");
}

/**
 * Определить из пути к исполняемому файлу его имя 
 *
 * \param argv параметр main()
 */

static void ckptFindBasename(char **argv)
{
	char           *basename = argv[0];

	strcpy(ckptExecutable, basename);

	basename = strrchr(basename, '/');

	if (basename == NULL)
		strcpy(ckptBasename, argv[0]);
	else
		strcpy(ckptBasename, basename + 1);
}

/**
 * Прочитать все настройки библиотеки.
 *
 * Настройки читаются сначала из файлов, потом из окружения
 *
 * \param argv параметр main()
 */

void ckptReadSetup(char **argv)
{
	char            buf[4096], *homedir;

	snprintf(buf, sizeof(buf), "%s/%s", ETCDIR, CONFFILE);
	ckptReadSetupFile(buf);

	homedir = getenv("HOME");
	if (homedir)
	{
		snprintf(buf, sizeof(buf), "%s/%s", homedir, HOME_CONFFILE);
		ckptReadSetupFile(buf);
	}
	ckptReadSetupEnv();
	ckptSetupDefaults();
	ckptFindBasename(argv);
}
