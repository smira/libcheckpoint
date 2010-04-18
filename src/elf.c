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
 * \file elf.c
 *
 * Чтение ELF-заголовков исполняемого файла
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

#ifdef HAVE_ELF_H
#include <elf.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "debug.h"
#include "system.h"

/**
 * Файловый дескриптор открытого исполняемого файла 
 */

static int      elf_fd;

/**
 * Основной ELF-заголовок 
 */

static ELF_EHDR elf_header;

/**
 * Время последней модификации исполняемого файла 
 */

static time_t 	elf_mtime;

/**
 * Открыть ELF-файл (исполняемый)
 *
 * \param filename имя исполняемого файла
 * \return 1, если всё успешно
 */

int ckptElfOpen(char *filename)
{
	struct stat sb;

	elf_fd = F_REAL_OPEN(filename, O_RDONLY, 0);
	if (elf_fd == -1)
		return 0;

	if (fstat(elf_fd, &sb) == -1)
	{
		F_REAL_CLOSE(elf_fd);
		return 0;
	}

	elf_mtime = sb.st_mtime;

	if (F_REAL_READ(elf_fd, &elf_header, sizeof(elf_header)) != sizeof(elf_header))
	{
		F_REAL_CLOSE(elf_fd);
		return 0;
	}
	F_REAL_LSEEK(elf_fd, elf_header.e_phoff, SEEK_SET);

	return 1;
}

/**
 * Закрыть ранее открытый при помощи ckptElfOpen файл 
 */

void ckptElfClose()
{
	F_REAL_CLOSE(elf_fd);
}

/**
 * Получить из ELF-файла информацию об очередной секции, которую требуется включить в КТ 
 *
 * \param addr здесь возвращается адрес начала секции в памяти
 * \param size здесь возвращается длина секции в байтах
 * \return 1, если всё успешно, 0 если не осталось больше секций (и в случае ошибки)
 */

int ckptElfGetSection(void **addr, size_t * size)
{
	static ELF_PHDR header;

	while (elf_header.e_phnum > 0)
	{
		if (F_REAL_READ(elf_fd, &header, sizeof(header)) != sizeof(header))
			return 0;

		elf_header.e_phnum--;

		if (header.p_type == PT_LOAD && ((header.p_flags & PF_W) == PF_W))
		{
			*addr = (void *)header.p_vaddr;
			*size = header.p_memsz;
			return 1;
		}
	}

	return 0;
}

/**
 * Получить время последней модификации ELF-файла
 *
 * \return время последней модификации
 */

time_t ckptElfGetMtime()
{
        return elf_mtime;
}
