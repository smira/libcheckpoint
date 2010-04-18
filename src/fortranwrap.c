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
 * \file fortranwrap.c
 *
 * Обертка для компиляции с программами на FORTRAN
 *
 * Управление попадает в функцию MAIN__, которая обозначает точку входа
 * в FORTRAN-программу, которая передает управление в точку входа библиотеки,
 * функцию __ckpt_entry, которая возвращает управление в ckpt_main, которая
 * вызывает фортрановскую функцию SUBROUTINE ckpt_main, которая является точкой входа в 
 * программу на Фортране
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * Указание на SUBROUTINE ckpt_main в FORTRAN-программе 
 */

extern int ckpt_main__(int argc, char **argv);

/**
 * Точка входа в библиотеку 
 */

extern int __ckpt_entry(int argc, char **argv);

/**
 * Основная функция библиотеки: создать КТ 
 */

extern int ckptCheckpoint();

/**
 * Макроопределения для обнаружения argc в Fortran 
 */

extern int FORTRAN_ARGC;                                                                   

/**
 * Макроопределения для обнаружения argv в Fortran 
 */

extern char **FORTRAN_ARGV;   

/**
 * Эта функция вызывается из __ckpt_entry для передачи управления в программу. 
 */

int ckpt_main(int argc, char **argv)
{
	return ckpt_main__(argc, argv);
}


/**
 * Интерфейс ckptCheckpoint() для фортрановских программ
 */

int ckptcheckpoint_()
{
	return ckptCheckpoint();
}

/**
 * Точка входа в Фортран-программу 
 */

int MAIN__(void)
{
	return __ckpt_entry(FORTRAN_ARGC, FORTRAN_ARGV);
}


#ifdef ENABLE_BENCHMARK

extern void ckptBenchmarkPrint();

/**
 * Обертка над ckptBenchmarkPrint для Fortran 
 */

void ckptbenchmarkprint_()
{
	ckptBenchmarkPrint();
}

#endif /* ENABLE_BENCHMARK */


