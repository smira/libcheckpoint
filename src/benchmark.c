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
 * \file benchmark.c
 *
 * Подсистема сбора статистики о продолжительности создания/восстановления КТ 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include "system.h"
#include "benchmark.h"
#include "setup.h"

#ifdef ENABLE_BENCHMARK

/**
 * Адрес начала области памяти, где хранятся записи статистики
 */

static void *benchmark_area;

/**
 * Записано в данный момент количество точек статистики     
 */

static int *benchmark_count;

/**
 * Запись статистики  
 */

struct benchmark_point
{
	int kind;                       /**< класс действия (битовая маска) */
	struct timeval stamp;           /**< временнАя метка действия */
};

/**
 * Указатель на начало списка структур статистики        
 */

struct benchmark_point *benchmark_points;

/**
 * Размер области памяти под статистику, байт
 */

#define BENCHMARK_AREA_SIZE	(sizeof(int) + MAX_BENCHMARK_POINTS*sizeof(struct benchmark_point))

/**
 * Получить класс действия 
 */

#define BENCH_POINT_KIND(k)	(((k) & 0xf0) >> 4)

/**
 * Получить тип действия : 0 - начало, 1 - конец  
 */

#define BENCH_POINT_TYPE(k)	((k) & 0xf)

/**
 *  Является ли действие интервальным? 
 */

#define BENCH_POINT_INTERVAL(k)	(((k) & 0x100) == 0x100)

/**
 * Строковые обозначения классов действий   
 */

static char *benchmark_kinds[] = 
{
	"Program",
	"Checkpoint",
	"Synchronization",
	"Restore",
};

/**
 * Инициализировать подсистему сбора статистики 
 *
 * \return 1, если успешно удалось выделить область памяти  
 */

int ckptBenchmarkInit()
{
	benchmark_area = F_REAL_MMAP(NULL, BENCHMARK_AREA_SIZE, PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_SHARED, -1, 0);
	benchmark_count = (int *)benchmark_area;
	benchmark_points = (struct benchmark_point *)(benchmark_count+1);
	*benchmark_count = 0;

	return benchmark_area != MAP_FAILED;			       
}

/**
 * Остановить систему сбора статистики
 *
 * \return 1, если всё успешно  
 */

int ckptBenchmarkDone()
{
	F_REAL_MUNMAP(benchmark_area, BENCHMARK_AREA_SIZE);

	return 1;
}

/**
 * Записать точку статистики в лог     
 *
 * \param kind класс действия 
 * \return 1, если всё успешно   
 */

int ckptBenchmarkPut(int kind)
{
	if (*benchmark_count >= MAX_BENCHMARK_POINTS)
		return 0;

	benchmark_points[*benchmark_count].kind = kind;
	gettimeofday(&benchmark_points[(*benchmark_count)++].stamp, (struct timezone *)NULL);

	return 1;
}

/**
 * Записать в поток стандартного вывода информацию о продолжительности различных этапов работы 
 * библиотеки.
 */

void ckptBenchmarkPrint()
{
	int i;
	int intervals[CKPT_BNCH_MAX_KIND+1];

	if (!ckptEnable)
		return;

	for (i = 0; i < CKPT_BNCH_MAX_KIND+1; i++)
		intervals[i] = -1;

	printf("==CHECKPOINT BENCHMARK START==\n");
	for (i = 0; i < *benchmark_count; i++)
	{
		if (BENCH_POINT_INTERVAL(benchmark_points[i].kind))
		{
			if (BENCH_POINT_TYPE(benchmark_points[i].kind) == 1)
				intervals[BENCH_POINT_KIND(benchmark_points[i].kind)] = i;
			else if (intervals[BENCH_POINT_KIND(benchmark_points[i].kind)] != -1)
			{
				double diff;
				int j = intervals[BENCH_POINT_KIND(benchmark_points[i].kind)];

				diff = (benchmark_points[i].stamp.tv_sec + 
						benchmark_points[i].stamp.tv_usec/1000000.0) -
					(benchmark_points[j].stamp.tv_sec +
						benchmark_points[j].stamp.tv_usec/1000000.0);

				printf("%20s INTERVAL = %4.6f sec\n",
					benchmark_kinds[BENCH_POINT_KIND(benchmark_points[i].kind)],
					diff);
				intervals[BENCH_POINT_KIND(benchmark_points[i].kind)] = -1;
			}
		}
		printf("%20s %8s @ %s", benchmark_kinds[BENCH_POINT_KIND(benchmark_points[i].kind)],
				BENCH_POINT_TYPE(benchmark_points[i].kind) == 1 ? "START" : "FINISH",
				ctime((time_t *)&benchmark_points[i].stamp.tv_sec));
	}		
	printf("==CHECKPOINT BENCHMARK FINISH==\n");
};

#endif /* ENABLE_BENCHMARK */

