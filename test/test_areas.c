/*
 * Test case for ../src/areas.c (.h) 
 */

#include <stdio.h>
#include "src/areas.h"

void PrintMemAreas()
{
	struct ckptMemArea *t;

	printf("--> Dump\n");
	for (t = ckptAreaHead; t != NULL; t = t->next)
		printf("%010p %08lx\n", t->start, t->size);
	printf("<-- Dump\n");
}

int ckpt_main(int argc, char *argv[])
{
	ckptIncludeMemArea((void *)0x12220, 0x200, 1);
	PrintMemAreas();
	ckptIncludeMemArea((void *)0x10000, 0x500, 1);
	ckptIncludeMemArea((void *)0x20000, 0x123, 1);
	ckptIncludeMemArea((void *)0x0FF00, 0x200, 1);
	ckptIncludeMemArea((void *)0x12300, 0x400, 1);
	ckptIncludeMemArea((void *)0x1FF00, 0x300, 1);
	ckptIncludeMemArea((void *)0x10300, 0x100, 1);
	ckptIncludeMemArea((void *)0x00000, 0x30000, 1);
	PrintMemAreas();
	
	/* SHOULD BE:
--> Dump
0x00000000 0000ff00
0x0000ff00 00000100
0x00010000 00000500
0x00010500 00001d20
0x00012220 00000200
0x00012420 000002e0
0x00012700 0000d800
0x0001ff00 00000100
0x00020000 00000123
0x00020123 000000dd
0x00020200 0000fe00
<-- Dump
	*/

	ckptExcludeMemArea((void *)0x00000, 0x30000);
	PrintMemAreas();

	/* SHOULD BE:
--> Dump
<-- Dump
	*/

	ckptIncludeMemArea((void *)0x20000, 0x05000, 1);
	ckptIncludeMemArea((void *)0x40000, 0x07000, 1);
	ckptExcludeMemArea((void *)0x21000, 0x21000);
	ckptExcludeMemArea((void *)0x43000, 0x01000);
	PrintMemAreas();
	
	/* SHOULD BE:
--> Dump
0x00020000 00001000
0x00042000 00001000
0x00044000 00003000
<-- Dump
	*/
	
	return 0;
}

