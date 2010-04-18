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
 * \file debug.c
 *
 * ¬спомогательные функции дл€ отладки и журналировани€ действий библиотеки
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "system.h"

#ifdef WITH_MPICH
#include "mpich/mpich.h"
#endif

/**
 * ќтображение типов важности сообщений библиотеки на приоритеты syslog 
 */

static int log_priority_map[10] =
{
	 /* 0 */ 0,
	 /* CKPT_LOG_ERROR */ LOG_ERR,
	 /* 2 */ 0,
	 /* CKPT_LOG_WARNING */ LOG_NOTICE,
	 /* 4 */ 0,
	 /* 5 */ 0,
	 /* 6 */ 0,
	 /* 7 */ 0,
	 /* 8 */ 0,
	 /* CKPT_LOG_DEBUG */ 0,
};

/**
 * «аписать сообщение в лог.
 *
 * ѕо умолчанию запись идЄт в стандартный поток ошибок и syslog
 *
 * \param priority прироритет сообщени€
 * \param message сообщение (формат printf)
 */

void ckptLog(int priority, const char *message,...)
{
	va_list         ap;
	static char     buf[10240];

	va_start(ap, message);

	
	if (log_priority_map[priority])
		/* 
                 * XXX: It's not safe to call vsyslog: it uses vnsprintf, which in turn
		 * uses malloc
		 */
	    	vsyslog(log_priority_map[priority], message, ap);

#ifdef WITH_MPICH
	snprintf(buf, sizeof(buf), "#%d:", ckptMpichNumber());
	F_REAL_WRITE(2, buf, strlen(buf));
#endif
	vsnprintf(buf, sizeof(buf), message, ap);
	F_REAL_WRITE(2, buf, strlen(buf));
	F_REAL_WRITE(2, "\n", 1);

	va_end(ap);
}
