/*-
 * Copyright (c) 2002-2007 Andrey Smirnov.  All rights reserved.
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
 * \file checkpoint.c
 * Основной модуль, запись контрольной точки.
 */

/**
 * \mainpage Библиотека libcheckpoint
 *
 * Библиотека позволяет создавать контрольные точки последовательных и параллельных (использующих библиотеку MPICH) задач.
 *
 * \section features Основные характеристики
  *
 * К основным характеристкам библиотеки можно отнести:
 *     - Реализация автоматических контрольных точек на уровне процесса пользователя.
 *     - Поддержка различных архитектур:
 *        - FreeBSD/x86
 *        - Linux/x86
 *        - Linux/Alpha 
 *     - Поддерживаемые языки программирования:
 *        - C/C++
 *        - Fortran 
 *     - Контрольные точки для параллельных задач, использующих библиотеку MPICH:
 *        - в коммуникационной среде Myrinet (MPICH-GM);
 *        - в коммуникационной среде Ethernet (ch_p4)
 *        - с разделяемой памятью (SMP). 
 *     - Сохранение состояния процесса:
 *        - сегмент данных;
 *        - стек;
 *        - куча (malloc/free);
 *        - контекст процесса (состояние регистров процессора). 
 *     - Сохранение состояния операционной системы:
 *        - отображения в память (mmap/munmap);
 *        - открытые файлы (open/close/lseek/read/write/...);
 *        - обработчики сигналов. 
 *     - Методы повышения эффективности:
 *        - поддержка асинхронных контрольных точек;
 *        - сжатие данных контрольной точки;
 *        - учет контрольных точек при программировании. 
 *     - Изменение параметров библиотеки во время выполнения (конфигурационный файл или переменные окружения).
 *     - Миграция процессов между машинами с идентичной архитектурой.
 *     - Получение информации об эффективности работы библиотеки.
 *
 * \section setup Установка библиотеки
 *
 * \subsection compile Сборка и установка
 *
 *  В данном разделе будет описан процесс сборки и установки библиотеки \c libcheckpoint. Для установки необходимо выполнить 
 *  следующие действия:
 *
 * <tt>tar xzf libcheckpoint-2.x.tar.gz \n
 * cd libcheckpoint-2.x/ \n
 * ./configure ... \n
 * make all \n
 * make install</tt>
 *
 * \subsection configure Параметры configure
 *
 *   - \c --with-mpich-dev=устройство \n
 *     Параметр определяет устройство MPICH, на которое должен быть настроена библиотека. 
 *     Если данный параметр не указан, библиотека собирается без подсистемы интерфейса с MPICH и 
 *     может использоваться для создания контрольных точек последовательных задач. Возможные значения: 
 *     \c ch_gm (Myrinet, МВС-1000М), \c ch_p4 (Ethernet), \c ch_shmem (разделяемая память, SMP).
 *
 *   - \c -with-mpich-dir=каталог, \c --with-gm-dir=каталог, \c --with-p4-dir=каталог \n
 *     Эти параметры задают расположение каталогов, в которые установлены библиотеки MPICH, 
 *     GM (для сети Myrinet) и исходники MPICH (для p4, Ethernet). Значения по умолчанию 
 *     подходят для большинства стандартных установок.
 *   
 *   - \c --with-max-procs=число \n
 *     Задает максимальное количество ветвей параллельной задачи (значение по умолчанию: 1024).
 *   
 *   - \c --with-max-mmaped=число \n
 *     Задает максимальное количество отображений в память, которое библиотека сможет сохранить и 
 *     восстановить. (Значение по умолчанию: 256.)
 *
 *   - \c --with-max-files=число \n
 *     Задает максимальное количество файловых дескрипторов, которое сможет сохранить библиотека. 
 *     (Значение по умолчанию: 128).    
 *
 *   - \c --with-max-mem-areas=число \n
 *     Задает максимальное количество фрагментов в списке областей адресного пространства процесса. 
 *     Для программ, активно использующих механизм ckptIncludeMem/ckptExcludeMem, может понадобиться 
 *     увеличение этого параметра. (Значение по умолчанию: 8192).
 *
 *   - \c --disable-compression \n
 *     Исключает из библиотеки подсистему сжатия контрольных точек.
 *
 *   - \c --disable-benchmarking \n
 *     Исключает из библиотеки подсистему сбора информации об эффективности.
 *
 * \subsection linking Подключение библиотеки КТ к программам
 *
 * \subsubsection linkc Для программ на языке С/C++
 *
 * В исходном коде программы необходимо изменить имя функции \c main() на \c ckpt_main(), для параллельных 
 * программ необходимо убрать обращения к функциям \c MPI_Init() и \c MPI_Finalize(). Эти действия произойдут автоматически, 
 * если в главный модуль программу включить заголовочный файл \c checkpoint.h, распространяемый вместе с библиотекой. 
 * Также необходимо в выбранных точках расположить обращения к функции создания контрольной точки \c ckptCheckpoint().
 *
 * Программа должна быть собрана статически с использованием библиотеки контрольных точек и специальными настройками 
 * (для перехвата обращением к библиотечным вызовам). Пример \c Makefile для осуществления такой сборки можно найти в 
 * каталоге \c test/mpi/ дистрибутива. 
 *
 * \subsubsection linkfortran Для программ на языке Fortran
 *
 * В исходном коде необходимо заменить \c 'PROGRAM \c xxx' на \c 'SUBROUTINE \c ckpt_main' и убрать обращения к 
 * \c MPI_Init() и \c MPI_Finalize(). Также в нужных местах добавляются вызовы \c ckptCheckpoint(). Сборка 
 * осуществляется с теми же параметрами, что и для программы на языке C, только следует использовать вариант библиотеки для 
 * языка Fortran \c libfcheckpoint.
 *
 * \section interface Интерфейс библиотеки
 *
 * Интерфейс библиотеки составляют следующие функции:
 *   - ckptCheckpoint()
 *   - ckptIncludeMem()        
 *   - ckptExcludeMem()        
 *   - ckptBenchmarkPrint()        
 *
 * \section tuning Настройка
 *
 * \subsection tuningvars Параметры библиотеки
 *  
 *  Список параметров библиотеки, в скобках - значение по умолчанию.
 *
 *   - \c enable (1) \n
 *      Включение/выключение подсистемы создания КТ.
 *      Если 0, то запрещается создание КТ и восстановление из неё. Библиотека 
 *      не функционирует.
 *   - \c directory (".")  \n 
 *      Каталог, в котором хранятся контрольные точки. По умолчанию они хранятся
 *      в текущем каталоге.
 *   - \c alarm_enable (0) \n  
 *      Включить автоматическое периодическое создание КТ. <b>Несовместимо с MPI.</b>
 *   - \c auto_period (600) \n   
 *      Интервал (в секундах) между автоматическими созданиями КТ.
 *   - \c hup_enable (1) \n 
 *      Создавать КТ по приходу сигнала SIGHUP? <b>Несовместимо с MPI.</b>
 *   - \c async_enable (0) \n 
 *      Асинхронное (фоновое) создание КТ при помощи fork(). <b>Несовместимо с MPI.</b>
 *   - \c mtime_check_disable (0) \n 
 *      Запретить проверку при восстановлении на неизменность времени 
 *      последней модификации исполняемого файла
 *   - \c checkpoints_in_subdirs (0) \n 
 *      Если 1, то КТ хранятся в подкаталогах в зависимости от номера хранимой КТ.
 *   - \c num_checkpoints_to_keep (1) \n 
 *      Количество наборов КТ, которые хранятся. Если 1, то хранится только последняя
 *      созданная КТ.
 *              
 * \subsection tuningfiles Файлы настройки
 *
 * Библиотека пытается прочитать сначала файл настройки <tt>%PREFIX%/etc/libcheckpoint.conf</tt>,
 * потом файл <tt>.libcheckpoint</tt> в домашнем каталоге текущего пользователя. Оба файла имеют
 * одинаковый формат:
 *
 * <tt>параметр=значение</tt>
 *
 * Например,
 *
 * <tt>
 * directory=/var/db/checkpoints \n                                                                                  
 * enable=1 \n                                                                                               
 * checkpoints_in_subdirs=0 \n                                                                               
 * num_checkpoints_to_keep=3                                                                               
 * </tt>
 *
 * \subsection tuningenv Настройка через переменные окружения
 *
 * Самый высокий приоритет имеют настройки, переданные через переменные окружения. Для передачи параметра
 * через переменную окружение его имя надо перевести в верхний регистр и добавить префикс \c CKPT_, например, для
 * Bourne-shell:
 *
 * <tt>CKPT_DIRECTORY=/var/db/checkpoints CKPT_ASYNC_ENABLE=1 ./program</tt>
 *
 * \section faq FAQ
 *
 * \subsection faq1 Проблемы при работе программ, собранных с библиотекой КТ
 *
 *   - <em>Почему программа завершается с 'Abort trap'?</em> \n
 *     Данное сообщение свидетельствует о том, что при работе библиотеки произошла фатальная ошибка, 
 *     которая не позволила продолжить выполнение программы. Обычно перед этой ошибкой библиотека выводит 
 *     более конкретное сообщение об ошибке. Более подробную информацию можно получить при помощи 
 *     backtrace из core-файла программы. 
 *
 * \subsection faq2 Подробности по устройству библиотеки
 *
 *   - <em>Как полно сохраняются открытые файлы?</em> \n
 *     Сохранение происходит при перехвате обращений к функциям системной библиотеки open, close, dup, dup2, 
 *     read, write, readv, writev. Если файловый дескриптор был получен в обход этих функций, библиотека 
 *     не сможет сохранить его состояние (так, не будет сохранены сокеты). Для файловых дескрипторов сохраняются 
 *     параметры, которые были переданы функции open, а также положение файлового указателя. Для файловых дескрипторов, 
 *     полученных при помощи вызовов dup/dup2 не сохраняется семантика 'общего' файлового указателя.
 *   - <em>Как полно сохраняются отображения в память?</em> \n
 *     Отображения в память сохраняются путём перехвата обращений к системным функциям mmap и munmap (для Linux 
 *     еще и к функциям __mmap и __munmap). Если отображение не было связано с файлом (параметр fd == -1), то в контрольной 
 *     точке сохраняется и содержимое области памяти. Если отображение было связано с файлом, и файловый дескриптор находится 
 *     под контролем библиотеки (см. предыдущий вопрос), файловый дескриптор будет сохранен, и отображение будет восстанавливаться 
 *     полноценно. Если же файловый дескриптор, связанный с отображением, не находится под контролем библиотеки, будет сохранено 
 *     содержимое области памяти, и будет восстановливаться без связи с файлом.
 *   - <em>Использует ли библиотека malloc(3)?</em> \n
 *     Нет, напрямую в библиотеке отсутствуют вызовы к динамическому распределения памяти. Такие обращения могут происходить 
 *     при вызове других функций стандартной библиотеки, например printf(3).
 *   - <em>Какой алгоритм используется для сжатия файлов контрольной точки?</em> \n
 *     XXX
 *
 * \section licence Лицензия
 *
 * Библиотека лицензирована по лицензии BSD. Часть её,
 * библиотека minilzo, распространяется по лицензии GPL, 
 * но может выключена при компиляции параметром \c --disable-compression.
 *
 * <tt>
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
 * </tt>
 *
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

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "setup.h"
#include "image.h"
#include "files.h"
#include "elf_.h"
#include "debug.h"
#include "system.h"
#include "vital.h"
#include "restore.h"
#include "areas.h"
#include "benchmark.h"

#ifdef WITH_MPICH
#include "mpich/mpich.h"
#endif

/**
 * Выровнять указатель на груницу страницы (в бОльшую сторону) 
 *
 * \param p указатель
 */

#define PAGE_ALIGN(p)	((void *)(((size_t)(p) + GPAGESIZE-1)/GPAGESIZE*GPAGESIZE))

/* Определения всех функций */ 
int                     ckptInit(int *argc, char ***argv);
int		        ckptDone();
static int              ckptInitAlarm();
static int              ckptInitSignal();
static int              ckptInitAsync();
static int	        ckptInitMemoryAreas();
int                     ckptCheckRestore();
static RETSIGTYPE       ckptSIGALRM_Handler(int);
static RETSIGTYPE       ckptSIGHUP_Handler(int);
static RETSIGTYPE       ckptSIGCHLD_Handler(int);
int                     ckptCheckpoint();
static int 	        ckptCheckpointHeader(ckptImage * image);
static int	        ckptCheckpointData(ckptImage * image, void *start, size_t size, int kind);
static int	        ckptCheckpointStack(ckptImage * image);

/**
 * XXX: Что это? 
 */

static void    *ckptOldBrk;

/**
 * Адрес символа - конец сегмента данных (UNIX) 
 */

extern int      end;

/* PID of the child used in asynchronous checkpointing */
static pid_t	child_pid;

extern int ckpt_main(int argc, char **argv); 

/* Override program entry point */
int
__ckpt_entry(int argc, char **argv)
{
	int result;
	
	ckptReadSetup(argv);

	if (ckptEnable)
		ckptInit(&argc, &argv);

	result = ckpt_main(argc, argv);
	
	if (ckptEnable)
		ckptDone();
		
	return result;
}

/* Initialise checkpointing engine */
int
ckptInit(int *argc, char ***argv)
{
#ifdef ENABLE_BENCHMARK
	if (!ckptBenchmarkInit())
		abort();
#endif
	
	CKPT_BENCHMARK(CKPT_BNCH_PROG_START);

#ifdef ENABLE_COMPRESSION
	if (!ckptImageInitCompression())
		abort();
#endif

	if (!ckptInitMemoryAreas())
		abort();

#ifdef WITH_MPICH
        /* Если работаем в параллельном режиме, создание
         * КТ через сигналы и асинхронно ненадежно 
         */
        ckptAlarmEnable = 0;
        ckptSignalEnable = 0;
        ckptAsyncEnable = 0;
#endif

	/* until we remember signals, we should do this first */
	if (ckptAlarmEnable)
		ckptInitAlarm();
	if (ckptSignalEnable)
		ckptInitSignal();
	if (ckptAsyncEnable)
		ckptInitAsync();

	
#ifdef  WITH_MPICH
	if (!ckptVitalInit())
		abort();

	if (!ckptMpichInit(argc, argv))
		abort();
#endif	

#ifdef 	WITH_MPICH
	if (ckptMpichCheckRestore())
#else
	if (ckptCheckRestore())
#endif
		ckptRestore();

	return 1;
}

/* Finalise checkpointing engine */
int
ckptDone()
{
#ifdef  WITH_MPICH
	ckptMpichDone();
	ckptVitalDone();
#endif	

#ifdef	ENABLE_COMPRESSION
	ckptImageDoneCompression();
#endif

	CKPT_BENCHMARK(CKPT_BNCH_PROG_FINISH);

#ifdef 	ENABLE_BENCHMARK
	ckptBenchmarkDone();
#endif
	
	return 1;
}

/* Initialise automatic checkpointing */
static int
ckptInitAlarm()
{
	signal(SIGALRM, ckptSIGALRM_Handler);
	alarm(ckptAlarmTimeout);
	return 1;
}

/* Initialise SIGHUP checkpointing */
static int
ckptInitSignal()
{
	signal(SIGHUP, ckptSIGHUP_Handler);
	return 1;
}

/* Initialise asynchronous checkpointing */
static int
ckptInitAsync()
{
	signal(SIGCHLD, ckptSIGCHLD_Handler);
	return 1;
}

/* Initialize memory areas, which should be automagically checkpointed */
static int
ckptInitMemoryAreas()
{
	void           *start;
	size_t          size;

	/* data sections & BSS, information via ELF header */
	if (ckptElfOpen(ckptExecutable))
	{
		while (ckptElfGetSection(&start, &size))
			ckptIncludeMemArea(start, size, CHNK_DATA);
		ckptElfClose();
	}
	else
		return 0;
	
	/* heap */
	start = HEAP_START; /* defined in config.h */
	ckptOldBrk = sbrk(0);
	if (ckptOldBrk > start)
		ckptIncludeMemArea(start, (char *)ckptOldBrk - (char *)start, CHNK_HEAP);
		
	return 1;
}

/* Check whether we should restore at this point */
int
ckptCheckRestore()
{
	char           *filename = ckptGetFinalFilename();
	int             result = ckptFileExists(filename);
	return result;
}

/* SIGALRM handler */
static RETSIGTYPE
ckptSIGALRM_Handler(int sig)
{
	ckptCheckpoint();
	signal(SIGALRM, ckptSIGALRM_Handler);
	alarm(ckptAlarmTimeout);
}

/* SIGHUP handler */
static RETSIGTYPE
ckptSIGHUP_Handler(int sig)
{
	ckptCheckpoint();
	signal(SIGHUP, ckptSIGHUP_Handler);
}

/* SIGCHLD handler */
static RETSIGTYPE
ckptSIGCHLD_Handler(int sig)
{
	int dummy;
	
	if (child_pid)
	{
	    if (waitpid(child_pid, &dummy, WNOHANG) == child_pid)
		child_pid = 0;
	}    
	
	signal(SIGCHLD, ckptSIGCHLD_Handler);
}

/**
 * Функция инициирует создание контрольной точки.
 *
 * При восстановлении из КТ всё происходит так, как если бы произошел возврат из неё. Данная функция 
 * является ключевой в интерфейсе библиотеки, её использование является обязательным. 
 *
 * \return 
 *    - 0 - успешное создание контрольной точки
 *    - 1 - возврат после восстановления
 *    - -1 - ошибка при создании контрольной точки
 */

int ckptCheckpoint()
{
	static int in_checkpointing = 0;
	
	/* engine not enabled */
	if (!ckptEnable)
	    return 0;
	
	/* already checkpointing, don't allow such calls */
	if (in_checkpointing)
	    return 0;
	
	/* previous asynchronous checkpoint hasn't been finished */
	if (ckptAsyncEnable && child_pid)
	    return 0;
	    
	/* handle asynchronous checkpointing and remember child pid */    
	if (ckptAsyncEnable)
	{  
	    child_pid = fork();
	    if (child_pid)
		return 0;
	}
	
	in_checkpointing = 1;

	CKPT_BENCHMARK(CKPT_BNCH_CKPT_BEGIN);
	
	/* remember signals and CPU registers */
	if (!sigsetjmp(ckptJumpBuffer,0xff))
	{
		char                   *temp_filename = ckptGetTempFilename();
		ckptImage       	image;
		struct ckptMemArea     *t;
		void		       *NewBrk;

#ifdef WITH_MPICH
		if (!ckptMpichBeforeCheckpoint())
                        return -1;
#endif
		ckptCreateImage(&image, temp_filename);

		ckptCheckpointHeader(&image);

		NewBrk = sbrk(0);
		if (NewBrk > ckptOldBrk)
		{
			ckptIncludeMemArea(ckptOldBrk, (char *)NewBrk - (char *)ckptOldBrk, CHNK_HEAP);
			ckptOldBrk = NewBrk;
		}
		else if (NewBrk < ckptOldBrk)
		{
			ckptExcludeMemArea(NewBrk, (char *)ckptOldBrk - (char *)NewBrk);
			ckptOldBrk = NewBrk;
		}

		/* memory areas */
		/* at first, CHNK_DATA */
		for (t = ckptAreaHead; t != NULL; t = t->next)
			if (t->kind == CHNK_DATA)
				ckptCheckpointData(&image, t->start, t->size, t->kind);
		/* then, all other */
		for (t = ckptAreaHead; t != NULL; t = t->next)
			if (t->kind != CHNK_DATA && t->kind != CHNK_EXCLUDE)
				ckptCheckpointData(&image, t->start, t->size, t->kind);

		/* stack */
		ckptCheckpointStack(&image);

		ckptCloseImage(&image);

		ckptRenameToFinal(temp_filename);

#ifdef WITH_MPICH		
		ckptMpichAfterCheckpoint();
#endif

		in_checkpointing = 0;

		CKPT_BENCHMARK(CKPT_BNCH_CKPT_END);
		
		if (ckptAsyncEnable)
		    _exit(0);
		
		return 0;
	}

	ckptRestoreFiles();
	ckptRestoreMmap(1);

	printf("<<< Restore finished!\n");

	CKPT_BENCHMARK(CKPT_BNCH_REST_END);
	in_checkpointing = 0;
	
	return 1;
}

/* Write checkpoint header */
static int
ckptCheckpointHeader(ckptImage * image)
{
	struct ckptCheckpointHdr header;

	header.brk = (void *) sbrk(0);
	header.stack_start = (void *)STACK_TOP;
	header.np = CKPT_NP;
	header.mtime = ckptElfGetMtime();

	return ckptWriteChunk(image, CHNK_HEADER, (size_t)sizeof(header), (void *)NULL, (void *)&header);
}

/* Write memory segment */
static int
ckptCheckpointData(ckptImage * image, void *start, size_t size, int kind)
{
	if (size == 0)
		return 1;

	return ckptWriteChunk(image, kind, size, start, start);
}

/* Write stack */
static int
ckptCheckpointStack(ckptImage * image)
{
	int             dummy;

	void           *start = (void *)STACK_TOP;
	size_t     	size = (size_t) ((char *)start - (char *)&dummy);

	return ckptWriteChunk(image, CHNK_STACK, size - STACK_GAP, (void *)NULL, (void *)((char *)start - size));
}


