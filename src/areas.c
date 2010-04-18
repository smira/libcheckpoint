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
 * \file areas.c
 * Манипуляции со списом областей памяти.
 *
 * Модуль учитывает распределение адресного пространства процесса под различные нужды:
 * куча, стек, сегмент данных, и т.п. Фактически определяет содержимое контрольной точки
 * процесса.
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include "areas.h"
#include "image.h"
#include "system.h"
#include "debug.h"

struct ckptMemArea *ckptAreaHead = NULL;                /**< Голова связанного списка областей памяти */
static struct ckptMemArea *ckptFreeHead = NULL;         /**< Голова связанного списка свободных структур информации об областях памяти */
static int ckptFreeBorder = 0;                          /**< Количество выделенных подряд структур struct ckptMemArea */
static struct ckptMemArea *ckptAreas = NULL;            /**< Начало области памяти хранения структур struct ckptMemArea */

static void ckptAreasInit();
static struct ckptMemArea *ckptAreasMalloc();
static void ckptAreasFree(struct ckptMemArea *p);

/**
 * Добавить информацию об области памяти к списку областей.
 *
 * При добавлении производится попытка создать
 * максимально компактный список областей памяти.
 *
 * \param _start адрес начала области
 * \param size размер области
 * \param kind тип области памяти
 */

void ckptIncludeMemArea(void *_start, size_t size, int kind)
{
	char *start = (char *)_start;

	struct ckptMemArea *t, *prev;
	struct ckptMemArea *new;

	if (size == 0)
		return;

	ckptAreasInit();

	for (prev = NULL, t = ckptAreaHead; t != NULL; prev = t, t = t->next)
	{
		/* case 1: new region is on the right of the current, continue search */
		if (start >= t->start+t->size)
			continue;
		/* case 2: new region is inside one of the existing, exiting  */
		if (t->start <= start && t->start+t->size >= start+size)
			return;
		/* case 3: right of new region is inside current */
		if (start+size > t->start && start+size <= t->start+t->size)
			size = t->start - start;
			/* FALLTHROUGH TO NEXT CASE */
		/* case 4: new region is on the left of the current, we should insert it on the left */
		if (start+size <= t->start)
		{
			new = ckptAreasMalloc();
			new->start = start;
			new->size = size;
			new->kind = kind;
			new->next = t;
			new->prev = t->prev;
			if (t->prev != NULL)
				t->prev->next = new;
			else
				ckptAreaHead = new;
			t->prev = new;
			return;
		}
		/* case 5: left of the new region is inside current */
		if (start > t->start && start < t->start+t->size)
		{
			size = size - (t->start+t->size-start);
			start = t->start+t->size;
			continue;
		}
		/* case 6: current region is inside new */
		if (start <= t->start && start+size >= t->start+t->size)
		{
			/* we add left part on the left, then leave right part as is */
			new = ckptAreasMalloc();
			new->start = start;
			new->size = t->start - start;
			new->kind = kind;
			new->next = t;
			new->prev = t->prev;
			if (t->prev != NULL)
				t->prev->next = new;
			else
				ckptAreaHead = new;
			t->prev = new;
			size = size - (t->start - start) - (t->size);
			start = t->start+t->size;
			t = new;
			continue;
		}
	}
	/* final case: we've passed through the list, and the place wasn't found, so insert it on the right */
	new = ckptAreasMalloc();
	new->start = start;
	new->size = size;
	new->kind = kind;
	new->next = NULL;
	new->prev = prev;
	if (new->prev == NULL)
		ckptAreaHead = new;
	else
		prev->next = new;
}

/**
 * Исключить информацию об области памяти
 *
 * В списке областей памяти вырезается "дырка" в указанной области,
 * про неё не будет сохранено никакой информации.
 *
 * \param _start адрес начала области
 * \param size размер области
 */

void ckptExcludeMemArea(void *_start, size_t size)
{
	char *start = (char *)_start;

	struct ckptMemArea *t, *prev;
	struct ckptMemArea *new;

	if (size == 0)
		return;

	for (prev = NULL, t = ckptAreaHead; t != NULL; prev = t, t = t->next)
	{
start:
		/* case 1: exclude region is on the right of the current, continue search */
		if (start >= t->start+t->size)
			continue;
		/* case 2: exclude region is on the left of the current, so we stop */
		if (start+size <= t->start)
			return;
		/* case 3: new region is inside current, slice current in two parts */
		if (t->start <= start && t->start+t->size >= start+size)
		{
			if (t->start < start)
			{
				new = ckptAreasMalloc();
				new->start = t->start;
				new->size = start - t->start;
				new->prev = t->prev;
				new->next = t;
				new->kind = t->kind;
				if (t->prev)
					t->prev->next = new;
				else
					ckptAreaHead = new;
				t->prev = new;
			}
			if (t->start+t->size > start+size)
			{
				new = ckptAreasMalloc();
				new->start = start+size;
				new->size = (t->start+t->size) - (start+size);
				new->prev = t;
				new->next = t->next;
				new->kind = t->kind;
				if (t->next)
					t->next->prev = new;
				t->next = new;
			}
			if (t->prev)
				t->prev->next = t->next;
			else
				ckptAreaHead = t->next;
			if (t->next)
				t->next->prev = t->prev;
			prev = t;
			t = t->next;
			ckptAreasFree(prev);
			if (t == NULL)
				return;
			goto start;
		}
		/* case 4: right of new region is inside current */
		if (start+size > t->start && start+size < t->start+t->size)
		{
			t->size = t->size - (start + size - t->start);
			t->start = start + size;
			return;
		}
		/* case 5: left of new region is inside current */
		if (start > t->start && start < t->start+t->size)
		{
			t->size = start - t->start;
			continue;
		}
		/* case 6: current region is inside exclusion */
		if (start <= t->start && start+size >= t->start+t->size)
		{
			if (t->prev)
				t->prev->next = t->next;
			else
				ckptAreaHead = t->next;
			if (t->next)
				t->next->prev = t->prev;
			prev = t;
			t = t->next;
			ckptAreasFree(prev);
			if (t == NULL)
				return;
			goto start;
		}
	}
}

/**
 * Инициализация подсистемы хранения областей памяти.
 *
 * Память выделяется при помощи mmap(), при выделении обходятся стандартные механизмы,
 * т.к. они используют сам механизм областей памяти.
 */

static void ckptAreasInit()
{
	if (ckptAreas != NULL)
		return;

	ckptAreas = (struct ckptMemArea *)F_REAL_MMAP(NULL, MAX_MEM_AREAS * sizeof(struct ckptMemArea), 
			PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (ckptAreas == MAP_FAILED)
	{
		ckptLog(CKPT_LOG_ERROR, "ckptAreasInit(): Unable to mmap memory for areas");
		_exit(2);
	}

	ckptAddMmaped(ckptAreas, MAX_MEM_AREAS * sizeof(struct ckptMemArea), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	ckptIncludeMemArea(ckptAreas, MAX_MEM_AREAS * sizeof(struct ckptMemArea), CHNK_INTERNAL);
}

/**
 * Выделение памяти под хранение информации об области памяти.
 *
 * Выделение идёт из памяти, выделенной в ckptAreasInit().
 *
 * \return указатель на выделенную память
 */

static struct ckptMemArea *ckptAreasMalloc()
{
	struct ckptMemArea *result;

	if (ckptFreeHead != NULL)
	{
		result = ckptFreeHead;
		ckptFreeHead = ckptFreeHead->next;
	}
	else
	{
		if (ckptFreeBorder >= MAX_MEM_AREAS) /* out of memory */
			_exit(3);
		result = ckptAreas + ckptFreeBorder++;
	}

	return result;
}

/**
 * Возвращает блок памяти с информацией об области памяти в список свободных.
 *
 * \param p указатель на освобождаемый блок памяти
 */

static void ckptAreasFree(struct ckptMemArea *p)
{
	p->next = ckptFreeHead;
	ckptFreeHead = p;
}

/**
 * Пользовательская функция: пометить область памяти как используемую (в качестве кучи).
 *
 * \param start адрес начала области
 * \param size размер области
 */

void ckptIncludeMem(void *start, size_t size)
{
	ckptExcludeMemArea(start, size);
	ckptIncludeMemArea(start, size, CHNK_HEAP);
}

/**
 * Пользовательская функция: пометить область памяти как неиспользуемую.
 *
 * \param start адрес начала области
 * \param size размер области
 */

void ckptExcludeMem(void *start, size_t size)
{
	ckptExcludeMemArea(start, size);
	ckptIncludeMemArea(start, size, CHNK_EXCLUDE);
}

/**
 * Распечатать текущий список областей памяти (отладочная функция). 
 */

void ckptPrintMemAreas()
{
	struct ckptMemArea *t;

	printf("--> Dump\n");
	for (t = ckptAreaHead; t != NULL; t = t->next)
		printf("%10p %08zx %d\n", t->start, t->size, t->kind);
	printf("<-- Dump\n");
}


