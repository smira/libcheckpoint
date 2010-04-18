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
 * \file checkpoint.h
 *
 * Основной заголовочный файл подсистемы создания КТ. 
 *
 * Должен включаться в пользовательскую программу.
 */

#ifndef __CKPT_CHECKPOINT_H__
#define __CKPT_CHECKPOINT_H__

#include <stdlib.h>

#ifdef __cplusplus                                                              
extern "C" {                                                                    
#endif                                                                          

/**
 * Создать контрольную точку немедленно
 *
 * \return возвращает:
 *   - 0: продолжение выполнения после создания КТ
 *   - 1: выполнение после восстановления из КТ 
 */

int ckptCheckpoint();

/**
 * Пользовательская функция: пометить область памяти как используемую (в качестве кучи).
 *
 * \param start адрес начала области
 * \param size размер области
 */

void ckptIncludeMem(void *start, size_t size);

/**
 * Пользовательская функция: пометить область памяти как неиспользуемую.
 *
 * \param start адрес начала области
 * \param size размер области
 */

void ckptExcludeMem(void *start, size_t size);

/**
 * В пользовательской программе прототип точки входа должен иметь вид:\n
 * <tt>int ckpt_main(int argc, char *argv[]);</tt>
 */

#define main ckpt_main

/**
 * Пользовательская программа не должна содержать обращений к MPI_Init 
 */

#define MPI_Init(a, b)	

/**
 * Пользовательская программа не должна содержать обращений к MPI_Finalize 
 */

#define MPI_Finalize()

#ifdef __cplusplus                                                              
} 
#endif                                                                          

#endif
