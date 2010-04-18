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
 * \file benchmark.h
 *
 * ���������� ����� ���������� � ����������������� ��������/�������������� �� 
 */

#ifndef __CKPT_BENCHMARK_H__
#define __CKPT_BENCHMARK_H__

/* ������ �������� */

/**
 * ������ ��������� (����� ��������)
 */

#define CKPT_BNCH_PROG_START		0x0001

/**
 * ���������� ��������� (����� ��������) 
 */

#define CKPT_BNCH_PROG_FINISH		0x0002

/**
 * ������ �������� ����������� ����� (����� ��������)
 */

#define CKPT_BNCH_CKPT_BEGIN		0x0111

/**
 * ��������� �������� ����������� ����� (����� ��������) 
 */

#define CKPT_BNCH_CKPT_END		0x0112

/**
 * ������ ������������� ������ ������������ ��������� (����� ��������) 
 */

#define CKPT_BNCH_SYNC_BEGIN		0x0121

/**
 * ��������� ������������� ������ ������������ ��������� (����� ��������) 
 */

#define CKPT_BNCH_SYNC_END		0x0122

/**
 * ������ �������������� �� ����������� ����� (����� ��������)  
 */

#define CKPT_BNCH_REST_BEGIN		0x0131

/**
 * ��������� �������������� �� ����������� ����� (����� ��������) 
 */

#define CKPT_BNCH_REST_END		0x0132

/**
 * ������������ ����� ������ �������� 
 */

#define CKPT_BNCH_MAX_KIND		  0x3

#ifdef ENABLE_BENCHMARK

/**
 * ������������ ���������� ������������ ����� ���������� 
 */

#define MAX_BENCHMARK_POINTS		128	/* so many because in any case it occupies whole page */

/**
 * ���������������� ���������� ����� ���������� 
 */

int ckptBenchmarkInit();

/**
 * ���������� ������� ����� ����������
 */

int ckptBenchmarkDone();

/**
 * �������� ����� � ��� ���������� 
 */

int ckptBenchmarkPut(int kind);

/**
 * �������� � ����� ������������ ������ ���������� � ����������������� ��������� ������ ������ 
 * ����������.
 */

void ckptBenchmarkPrint();

/**
 * �������� ����� � ��� ������������� (������) 
 */

#define CKPT_BENCHMARK(k)			ckptBenchmarkPut(k)

#else

#define CKPT_BENCHMARK(k)

#endif /* ENABLE_BENCHMARK */

#endif /* __CKPT_BENCHMARK_H__ */

