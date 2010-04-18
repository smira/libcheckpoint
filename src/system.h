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
 * \file system.h
 *
 * Сохранение состояния ОС: отображения в память и файловые дескрипторы
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

/**
 * Не перехватывать обращения к системным функциям 
 */

extern int ckptSystemIgnore;

#ifdef F_WRAP_MALLOC
extern int ckptMallocExclude;
#endif

/**
 * Восстановить карту отображения в память
 */

int ckptRestoreMmap(int restore_file_backed);

/**
 * Восстановить файловые дескрипторы 
 */

int ckptRestoreFiles();

/**
 * Уничтожить все запомненные отображения в память перед началом процедуры восстановления 
 */

void ckptPurgeMmaped();

/**
 * Добавить новую отображаемую область 
 */

void ckptAddMmaped(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

/**
 *  Удалить информацию об отображаемой области
 */
void ckptDelMmaped(void *addr, size_t size);

void *F_REAL_MMAP(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int F_REAL_MUNMAP(void *addr, size_t len);
int F_REAL_OPEN(const char *path, int flags, int mode);
int F_REAL_CLOSE(int d);
ssize_t F_REAL_READ(int d, void *buf, size_t nbytes);
ssize_t F_REAL_WRITE(int d, void *buf, size_t nbytes);
ssize_t F_REAL_READV(int d, const struct iovec *iov, int iovcnt);
ssize_t F_REAL_WRITEV(int d, const struct iovec *iov, int iovcnt);
off_t F_REAL_LSEEK(int fildes, off_t offset, int whence);

#endif /* __SYSTEM_H__ */

