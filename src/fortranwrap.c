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
 * ������� ��� ���������� � ����������� �� FORTRAN
 *
 * ���������� �������� � ������� MAIN__, ������� ���������� ����� �����
 * � FORTRAN-���������, ������� �������� ���������� � ����� ����� ����������,
 * ������� __ckpt_entry, ������� ���������� ���������� � ckpt_main, �������
 * �������� ������������� ������� SUBROUTINE ckpt_main, ������� �������� ������ ����� � 
 * ��������� �� ��������
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * �������� �� SUBROUTINE ckpt_main � FORTRAN-��������� 
 */

extern int ckpt_main__(int argc, char **argv);

/**
 * ����� ����� � ���������� 
 */

extern int __ckpt_entry(int argc, char **argv);

/**
 * �������� ������� ����������: ������� �� 
 */

extern int ckptCheckpoint();

/**
 * ���������������� ��� ����������� argc � Fortran 
 */

extern int FORTRAN_ARGC;                                                                   

/**
 * ���������������� ��� ����������� argv � Fortran 
 */

extern char **FORTRAN_ARGV;   

/**
 * ��� ������� ���������� �� __ckpt_entry ��� �������� ���������� � ���������. 
 */

int ckpt_main(int argc, char **argv)
{
	return ckpt_main__(argc, argv);
}


/**
 * ��������� ckptCheckpoint() ��� ������������� ��������
 */

int ckptcheckpoint_()
{
	return ckptCheckpoint();
}

/**
 * ����� ����� � �������-��������� 
 */

int MAIN__(void)
{
	return __ckpt_entry(FORTRAN_ARGC, FORTRAN_ARGV);
}


#ifdef ENABLE_BENCHMARK

extern void ckptBenchmarkPrint();

/**
 * ������� ��� ckptBenchmarkPrint ��� Fortran 
 */

void ckptbenchmarkprint_()
{
	ckptBenchmarkPrint();
}

#endif /* ENABLE_BENCHMARK */


