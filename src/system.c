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
 * \file system.c
 *
 * ���������� ��������� ��: ����������� � ������ � �������� �����������
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

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#include "system.h"
#include "image.h"
#include "areas.h"
#include "debug.h"

/**
 * �� ������������� ��������� � ��������� �������� 
 */

int ckptSystemIgnore = 0;

/*
 *
 * ������ � ��������� ������������� (open, close, dup, dup2, lseek, read, write, readv, writev)
 *
 */

/*
 * Leave some room for file descriptors of maps
 */

/**
 * ����� ���������� �������� ������������, ��� ������� ����������� ��������� 
 */

#define _MAX_FILES	((MAX_FILES) + (MAX_MMAPED))

/**
 * ���� �� ��� ������������������� ���������� ��������� ��������� � ������? 
 */

static int ckptFileInitialized = 0;

/**
 * ����������� �������� ���������� -> ���� �� ������� �� ���������.
 *
 * ���������� � ������ �������� � ckptFiles, ckptFileMap ������� ���������
 * ������� �������� ������������.
 */

static int ckptFilesMap[_MAX_FILES];

/**
 * ���������� �� �������� ������ 
 */

static struct
{
	int in_use;                     /**< ������������ �� ������ */
        int ref_cnt;                    /**< ���������� ������ �� ������ ���� */
        int fd_ref;                     /**< ���� != -1, �� ��� ������ �� �������� ���������� #fd_ref */
	char path[1024];                /**< ���� � ��������� ����� */
	off_t position;                 /**< ��������� ��������� ��������� */
	int flags;                      /**< ����� �������� ����� */
	int mode;                       /**< ����� �������� ����� */
        int restored_fd;                /**< ��� �������������� - fd, ���� ������ ��� ���� ������������� */
} ckptFiles[_MAX_FILES];

/**
 * ��������� open() 
 */

int F_REAL_OPEN(const char *path, int flags, int mode);

/**
 * ��������� close() 
 */

int F_REAL_CLOSE(int d);

/**
 * ��������� dup() 
 */

int F_REAL_DUP(int oldd);

/**
 * ��������� dup2() 
 */

int F_REAL_DUP2(int oldd, int newd);

/**
 * ��������� read() 
 */

ssize_t F_REAL_READ(int d, void *buf, size_t nbytes);

/**
 * ��������� write() 
 */

ssize_t F_REAL_WRITE(int d, void *buf, size_t nbytes);

/**
 * ��������� readv() 
 */

ssize_t F_REAL_READV(int d, const struct iovec *iov, int iovcnt);

/**
 * ��������� writev() 
 */

ssize_t F_REAL_WRITEV(int d, const struct iovec *iov, int iovcnt);

/**
 * ��������� lseek() 
 */

off_t F_REAL_LSEEK(int fildes, off_t offset, int whence);

/**
 * ������������������� ������� �������� ��������� � ������ 
 */

static void ckptFilesInitialize()
{
        if (ckptFileInitialized)
                return;

        memset(&ckptFilesMap, 0xff, sizeof(ckptFilesMap));

        ckptFileInitialized = 1;
}

/**
 * ������� ��� open() 
 */

int F_WRAP_OPEN(const char *path, int flags, int mode)
{
	int ret;

        ckptFilesInitialize();

	ret = F_REAL_OPEN(path, flags, mode);
        if (!ckptSystemIgnore && ret >= 0)
        {
                int i;

                assert(ret < MAX_FILES);
                assert(ckptFilesMap[ret] == -1);
                
                for (i = 0; i < MAX_FILES; i++)
                        if (!ckptFiles[i].in_use)
                                break;

                assert(i != MAX_FILES);

		ckptFiles[i].in_use = 1;
                ckptFiles[i].ref_cnt = 1;
                ckptFiles[i].fd_ref = -1;
		strcpy(ckptFiles[i].path, path);
		ckptFiles[i].flags = flags & ~(O_CREAT | O_TRUNC);
		ckptFiles[i].mode = mode;
		ckptFiles[i].position = F_REAL_LSEEK(ret, 0, SEEK_CUR);
                ckptFilesMap[ret] = i;
        }

	return ret;
}

/**
 * ������� ��� close() 
 */

int F_WRAP_CLOSE(int d)
{
	int ret;

	ret = F_REAL_CLOSE(d);
	if (!ckptSystemIgnore && ret >= 0)
        {
                assert(d < MAX_FILES);

                if (ckptFilesMap[d] != -1)
                {
                        ckptFiles[ckptFilesMap[d]].ref_cnt--;
                        if (ckptFiles[ckptFilesMap[d]].ref_cnt == 0)
                                ckptFiles[ckptFilesMap[d]].in_use  = 0;
                        ckptFilesMap[d] = -1;
                }
        }

	return ret;
}

/**
 * ������� ��� dup()
 */

int F_WRAP_DUP(int oldd)
{
	int ret;

	ret = F_REAL_DUP(oldd);
	if (!ckptSystemIgnore && ret >= 0)
	{
                assert(ret < MAX_FILES);
                assert(ckptFilesMap[ret] == -1);

                if (ckptFilesMap[oldd] == -1)
                {
                        int i;

                        for (i = 0; i < MAX_FILES; i++)
                                if (!ckptFiles[i].in_use)
                                        break;

                        assert(i != MAX_FILES);

                        ckptFiles[i].in_use = 1;
                        ckptFiles[i].ref_cnt = 1;
                        ckptFiles[i].fd_ref = oldd;
                        ckptFilesMap[ret] = i;
                }
                else
                {
                        ckptFilesMap[ret] = ckptFilesMap[oldd];
                        ckptFiles[ckptFilesMap[ret]].ref_cnt++;
                }
	}

	return ret;
}

/**
 * ������� ��� dup2() 
 */

int F_WRAP_DUP2(int oldd, int newd)
{
	int ret;

	ret = F_REAL_DUP2(oldd, newd);
	if (!ckptSystemIgnore && ret >= 0)
	{
                assert(ret < MAX_FILES);

                if (ckptFilesMap[ret] != -1) 
                {
                        ckptFiles[ckptFilesMap[ret]].ref_cnt--;
                        if (ckptFiles[ckptFilesMap[ret]].ref_cnt == 0)
                                ckptFiles[ckptFilesMap[ret]].in_use  = 0;
                }

                if (ckptFilesMap[oldd] == -1)
                {
                        int i;

                        for (i = 0; i < MAX_FILES; i++)
                                if (!ckptFiles[i].in_use)
                                        break;

                        assert(i != MAX_FILES);

                        ckptFiles[i].in_use = 1;
                        ckptFiles[i].ref_cnt = 1;
                        ckptFiles[i].fd_ref = oldd;
                        ckptFilesMap[ret] = i;
                }
                else
                {
                        ckptFilesMap[ret] = ckptFilesMap[oldd];
                        ckptFiles[ckptFilesMap[ret]].ref_cnt++;
                }
	}

	return ret;
}

/**
 * ������� ��� read() 
 */

ssize_t F_WRAP_READ(int d, void *buf, size_t nbytes)
{
	ssize_t ret;

	ret = F_REAL_READ(d, buf, nbytes);
	if (!ckptSystemIgnore && ret > 0)
        {
                assert(d < MAX_FILES);
                
                if (ckptFilesMap[d] != -1)
                {
                        if (ckptFiles[ckptFilesMap[d]].fd_ref == -1)
                                ckptFiles[ckptFilesMap[d]].position += ret;
                }

        }

	return ret;
}

/**
 * ������� ��� write() 
 */

ssize_t F_WRAP_WRITE(int d, void *buf, size_t nbytes)
{
	ssize_t ret;

	ret = F_REAL_WRITE(d, buf, nbytes);
	if (!ckptSystemIgnore && ret > 0)
        {
                assert(d < MAX_FILES);
                
                if (ckptFilesMap[d] != -1)
                {
                        if (ckptFiles[ckptFilesMap[d]].fd_ref == -1)
                                ckptFiles[ckptFilesMap[d]].position += ret;
                }

        }

	return ret;
}

/**
 * ������� ��� readv() 
 */

ssize_t F_WRAP_READV(int d, const struct iovec *iov, int iovcnt)
{
	ssize_t ret;

	ret = F_REAL_READV(d, iov, iovcnt);
	if (!ckptSystemIgnore && ret > 0)
        {
                assert(d < MAX_FILES);
                
                if (ckptFilesMap[d] != -1)
                {
                        if (ckptFiles[ckptFilesMap[d]].fd_ref == -1)
                                ckptFiles[ckptFilesMap[d]].position += ret;
                }

        }

	return ret;
}

/**
 * ������� ��� writev() 
 */

ssize_t F_WRAP_WRITEV(int d, const struct iovec *iov, int iovcnt)
{
	ssize_t ret;

	ret = F_REAL_WRITEV(d, iov, iovcnt);
	if (!ckptSystemIgnore && ret > 0)
        {
                assert(d < _MAX_FILES);
                
                if (ckptFilesMap[d] != -1)
                {
                        if (ckptFiles[ckptFilesMap[d]].fd_ref == -1)
                                ckptFiles[ckptFilesMap[d]].position += ret;
                }

        }

	return ret;
}

/**
 * ������� ��� lseek() 
 */

off_t F_WRAP_LSEEK(int fildes, off_t offset, int whence)
{
	off_t ret;

	ret = F_REAL_LSEEK(fildes, offset, whence);
	if (!ckptSystemIgnore && ret >= 0)
        {
                assert(fildes < _MAX_FILES);
                
                if (ckptFilesMap[fildes] != -1)
                {
                        if (ckptFiles[ckptFilesMap[fildes]].fd_ref == -1)
                                ckptFiles[ckptFilesMap[fildes]].position = ret;
                }

        }

	return ret;
}

/**
 * ������������ �������� ����������� 
 */

int ckptRestoreFiles()
{
	int i;

	for (i = 0; i < _MAX_FILES; i++)
		if (ckptFiles[i].in_use)
                        ckptFiles[i].restored_fd = ckptFiles[i].fd_ref;

	for (i = 0; i < _MAX_FILES; i++)
		if (ckptFilesMap[i] != -1)
		{
			int fd;

                        if (ckptFiles[ckptFilesMap[i]].restored_fd == -1)
                        {
                                ckptFiles[ckptFilesMap[i]].restored_fd = F_REAL_OPEN(ckptFiles[ckptFilesMap[i]].path, ckptFiles[ckptFilesMap[i]].flags, 
                                                ckptFiles[ckptFilesMap[i]].mode);
                                assert(ckptFiles[ckptFilesMap[i]].restored_fd >= 0);
                                if ((ckptFiles[ckptFilesMap[i]].flags & O_APPEND) == 0)
                                {
                                        off_t res = F_REAL_LSEEK(ckptFiles[ckptFilesMap[i]].restored_fd, ckptFiles[ckptFilesMap[i]].position, SEEK_SET);
                                        assert(res == ckptFiles[ckptFilesMap[i]].position);
                                }

                                if (ckptFiles[ckptFilesMap[i]].restored_fd != i)
                                {
                                        fd = F_REAL_DUP2(ckptFiles[ckptFilesMap[i]].restored_fd, i);
                                        assert(fd == i);
                                        close(ckptFiles[ckptFilesMap[i]].restored_fd);
                                        ckptFiles[ckptFilesMap[i]].restored_fd = i;
                                }
                        }
                        else
                        {
                                fd = F_REAL_DUP2(ckptFiles[ckptFilesMap[i]].restored_fd, i);
                                assert(fd == i);
                        }
		}

	return 1;
}

#ifdef F_WRAP_MALLOC

/*
 * malloc/calloc/realloc/free
 *
 * ������������ ������ ��� mpich_gm
 */

int ckptMallocExclude = 0;

void *F_REAL_MALLOC(size_t size);
void *F_REAL_CALLOC(size_t size);
void *F_REAL_REALLOC(void *ptr, size_t size);
void F_REAL_FREE(void *ptr);

static struct
{
	void *ptr;
	size_t size;
} ckptMallocZones[1024];

void *F_WRAP_MALLOC(size_t size)
{
	void *ptr;

	if (ckptMallocExclude && size < 0x100)
		size = 0x100;

	ptr = F_REAL_MALLOC(size);
	if (ckptMallocExclude && ptr && size > 0x1000)
	{
		int i;

		ckptExcludeMemArea(ptr, size);
		ckptIncludeMemArea(ptr, size, CHNK_EXCLUDE);

		for (i = 0; i < 1024; i++)
			if (ckptMallocZones[i].ptr == NULL)
			{
				ckptMallocZones[i].ptr = ptr;
				ckptMallocZones[i].size = size;
				break;
			}
	}

	return ptr;
}

void *F_WRAP_CALLOC(size_t size)
{
	void *ptr;

	if (ckptMallocExclude && size < 0x100)
		size = 0x100;

	ptr = F_REAL_CALLOC(size);
	if (ckptMallocExclude && ptr && size > 0x1000)
	{
		int i;

		ckptExcludeMemArea(ptr, size);
		ckptIncludeMemArea(ptr, size, CHNK_EXCLUDE);

		for (i = 0; i < 1024; i++)
			if (ckptMallocZones[i].ptr == NULL)
			{
				ckptMallocZones[i].ptr = ptr;
				ckptMallocZones[i].size = size;
				break;
			}
	}

	return ptr;
}

void *F_WRAP_REALLOC(void *ptr, size_t size)
{
	void *newptr;

#if 0
	if (ckptMallocExclude && size < 0x100)
		size = 0x100;
#endif

	newptr = F_REAL_REALLOC(ptr, size);

#if 0
	if (ckptMallocExclude && newptr)
	{
		int i;

		for (i = 0; i < 1024; i++)
			if (ckptMallocZones[i].ptr == ptr)
			{
				ckptIncludeMemArea(ckptMallocZones[i].ptr, ckptMallocZones[i].size,
						CHNK_HEAP);
				ckptMallocZones[i].ptr = newptr;
				ckptMallocZones[i].size = size;
				break;
			}
		ckptExcludeMemArea(newptr, size);
		ckptIncludeMemArea(newptr, size, CHNK_EXCLUDE);
	}
#endif

	return newptr;
}

void F_WRAP_FREE(void *ptr)
{
	int i;

	if (ckptMallocExclude)
		for (i = 0; i < 1024; i++)
			if (ckptMallocZones[i].ptr == ptr)
			{
				ckptIncludeMemArea(ckptMallocZones[i].ptr, ckptMallocZones[i].size,
						CHNK_HEAP);
				ckptMallocZones[i].ptr = NULL;
				break;
			}
	return F_REAL_FREE(ptr);
}

#endif /* F_WRAP_MALLOC */

/*
 * 
 * �������� mmap/munmap 
 *
 */


/**
 * ����������� �������, ������������ � ������ 
 */

static struct
{
    void *addr;                         /**< ����� ������ ������� */
    size_t len;                         /**< ����� ������� */
    int prot;                           /**< �������� mmap */
    int flags;                          /**< �������� mmap */        
    int fd;                             /**< �������� ���������� ������������� ����� */
    off_t offset;                       /**< �������� ����������� ������������ ������ ����� */
} ckptMmaped[MAX_MMAPED];

/**
 * �������� ����� ������������ ������� 
 *
 * \param addr ����� �������
 * \param len ������ �������
 * \param prot �������� mmap
 * \param flags �������� mmap
 * \param fd �������� ����������
 * \param offset ��������
 * \callergraph
 */

void ckptAddMmaped(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    int i;
    
    for (i = 0; i < MAX_MMAPED; i++)
	if (ckptMmaped[i].addr == NULL)
                break;

    assert(i != MAX_MMAPED);

    ckptMmaped[i].addr = addr;
    ckptMmaped[i].len = len;
    ckptMmaped[i].prot = prot;
    ckptMmaped[i].flags = flags;
    ckptMmaped[i].fd = fd;
    ckptMmaped[i].offset = offset;
    if (ckptMmaped[i].fd != -1)
    {
            assert(ckptMmaped[i].fd < MAX_FILES);
            if (ckptFilesMap[ckptMmaped[i].fd] != -1)
            {
                    int j;
                    for (j = MAX_FILES; j < _MAX_FILES; j++)
                            if (ckptFilesMap[j] == -1)
                                    break;

                    assert(j != _MAX_FILES);

                    assert(ckptFiles[j].in_use == 0);
                    ckptFiles[j] = ckptFiles[ckptFilesMap[fd]];
                    ckptFiles[j].restored_fd = -1;
                    ckptFiles[j].ref_cnt = 1;
                    ckptFilesMap[j] = j;
                    ckptMmaped[i].fd = j;
            }
            else
            {
                    /* we haven't intercepted any fd, so do our best and save memory block */
                    ckptMmaped[i].fd = -1;
                    ckptMmaped[i].flags |= MAP_ANON;
                    ckptMmaped[i].offset = 0;
                    ckptIncludeMemArea(addr, len, CHNK_MMAP);
            }
    }
}

/**
 *  ������� ���������� �� ������������ �������
 *
 * \callergraph
 */

void ckptDelMmaped(void *addr, size_t size)
{
    int i;
    
    for (i = 0; i < MAX_MMAPED; i++)
    {
	if (ckptMmaped[i].addr == addr)
	{
	    ckptMmaped[i].addr = NULL;
	    ckptMmaped[i].len = 0;
	    if (ckptMmaped[i].fd != -1 && ckptFilesMap[ckptMmaped[i].fd] != -1)
            {
                    if (ckptFiles[ckptFilesMap[ckptMmaped[i].fd]].restored_fd != -1)
                            F_WRAP_CLOSE(ckptMmaped[i].fd);
		    ckptFiles[ckptFilesMap[ckptMmaped[i].fd]].in_use = -0;
		    ckptFilesMap[ckptMmaped[i].fd] = -1;
            }
	    break;
	}
    }
}


/**
 * ��������� mmap()
 */

void *F_REAL_MMAP(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

/**
 * ������� ��� mmap() 
 */

void *F_WRAP_MMAP(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    void *ret;
    
    ret = F_REAL_MMAP(addr, len, prot, flags, fd, offset);    
    
    if (!ckptSystemIgnore && ret != MAP_FAILED)
	ckptAddMmaped(ret, len, prot, flags, fd, offset);

    if (!ckptSystemIgnore && fd == -1 && ret != MAP_FAILED)
    {
	ckptIncludeMemArea(ret, len, CHNK_MMAP);
    }
    return ret;
}

#ifdef F_WRAP_MMAP2

/**
 * ��������� __mmap (Linux)
 */

void *F_REAL_MMAP2(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

/**
 * ������� ��� __mmap (Linux)
 */

void *F_WRAP_MMAP2(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    void *ret;
    
    ret = F_REAL_MMAP2(addr, len, prot, flags, fd, offset);    
    
    if (!ckptSystemIgnore && ret != MAP_FAILED)
	ckptAddMmaped(ret, len, prot, flags, fd, offset);

    if (!ckptSystemIgnore && fd == -1 && ret != MAP_FAILED)
    {
	ckptIncludeMemArea(ret, len, CHNK_MMAP);
    }
    return ret;
}
#endif

/**
 * ��������� munmap() 
 */

int F_REAL_MUNMAP(void *addr, size_t len);

/**
 * ������� ��� munmap() 
 */

int F_WRAP_MUNMAP(void *addr, size_t len)
{
    if (!ckptSystemIgnore)
    {
	    ckptExcludeMemArea(addr, len);
	    ckptDelMmaped(addr, len);
    }
    return F_REAL_MUNMAP(addr, len);
}

#ifdef F_WRAP_MUNMAP2

/**
 * ��������� __munmap() 
 */

int F_REAL_MUNMAP2(void *addr, size_t len);

/**
 * ������� ��� __munmap() 
 */

int F_WRAP_MUNMAP2(void *addr, size_t len)
{
    if (!ckptSystemIgnore)
    {
	    ckptExcludeMemArea(addr, len);
	    ckptDelMmaped(addr, len);
    }
    return F_REAL_MUNMAP2(addr, len);
}
#endif

/**
 * ���������� ��� ����������� ����������� � ������ ����� ������� ��������� �������������� 
 *
 * \callergraph
 */

void ckptPurgeMmaped()
{
	int i;
	for (i = 0; i < MAX_MMAPED; i++)
	{
		if (ckptMmaped[i].addr != NULL)
		{
			F_REAL_MUNMAP(ckptMmaped[i].addr, ckptMmaped[i].len);
		}
	}
}


/**
 * ������������ ����� ����������� � ������
 *
 * \param restore_file_backed ��������������� ����������� �� ����� ��� ��� ������
 * \callergraph
 */

int ckptRestoreMmap(int restore_file_backed)
{
    int i;
    for (i = 0; i < MAX_MMAPED; i++)
    {
	if (ckptMmaped[i].addr != NULL)
	{
	    void *addr;

	    if ( restore_file_backed && ckptMmaped[i].fd == -1)
		    continue;
	    if (!restore_file_backed && ckptMmaped[i].fd != -1)
		    continue;
	    addr = F_REAL_MMAP(ckptMmaped[i].addr, ckptMmaped[i].len, 
	                    ckptMmaped[i].prot, ckptMmaped[i].flags, 
		  	    ckptMmaped[i].fd, ckptMmaped[i].offset);
	    if ( addr != ckptMmaped[i].addr)
		abort();
	}
    }

    return 1;
}

