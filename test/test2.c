/*
 * Advanced test for checkpoint library
 * We are testing:
 * 	* memory exclusion/inclusion
 * 	* file operation
 * 	* mmaping
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/checkpoint.h"
#include "../src/areas.h"
#include "../src/benchmark.h"

char exclusion[4] = "aaa";

void PrintMemAreas()
{
	struct ckptMemArea *t;

	printf("--> Dump\n");
	for (t = ckptAreaHead; t != NULL; t = t->next)
		printf("%010p %08lx %d\n", t->start, t->size, t->kind);
	printf("<-- Dump\n");
}

int main(int argc, char **argv)
{
	const char *pattern = "abcdefgh";
	char buf[5];
	char *mmaped, *mmaped2;
	char *malloced;
	int i;
	int fd, fd2, fd3, fd4;

	fd = open("/tmp/ckpt_test", O_RDWR | O_CREAT, 0644);
	if (fd < 0)
	{
		perror("open");
		return 1;
	}

	write(fd, pattern, strlen(pattern));
	
	lseek(fd, 4, SEEK_SET);

        fd4 = dup(fd);
	fd3 = dup(fd);
	
        close(fd4);

	fd2 = open("/tmp/ckpt_test", O_RDONLY);
	if (fd2 < 0)
	{
		perror("open");
		return 1;
	}

	mmaped = mmap(NULL, strlen(pattern)*128, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
	if (mmaped == MAP_FAILED)
	{
		perror("mmap");
		return 1;
	}
	
	for (i = 0; i < 128; i++)
		memcpy(mmaped + i*strlen(pattern), pattern, strlen(pattern));


	malloced = malloc(strlen(pattern)*128);
	for (i = 0; i < 128; i++)
		memcpy(malloced + i*strlen(pattern), pattern, strlen(pattern));

	mmaped2 = mmap(NULL, strlen(pattern), PROT_READ, MAP_SHARED, fd2, 0);
	if (mmaped2 == MAP_FAILED)
	{
		perror("mmap");
		return 1;
	}

	close(fd2);

	PrintMemAreas();
	strcpy(exclusion, "bbb");
	ckptExcludeMem(exclusion, sizeof(exclusion));
	PrintMemAreas();

	if (ckptCheckpoint() == 0)
	{
		ckptBenchmarkPrint();
		return 0;
	}
	
	if (lseek(fd3, 0, SEEK_CUR) != 4)
		printf("File1 test: FAILED\n");
	else
		printf("File1 test: OK\n");

	read(fd, buf, 4);
	buf[4] = '\0';

	if (strcmp(buf, pattern+4) != 0)
		printf("File2 test: FAILED\n");
	else
		printf("File2 test: OK\n");

	
	if (memcmp(mmaped, pattern, strlen(pattern)) != 0 ||
	    memcmp(mmaped+87*strlen(pattern), pattern, strlen(pattern)) != 0)
		printf("Mmap test: FAILED\n");
	else
		printf("Mmap test: OK\n");

	if (memcmp(malloced, pattern, strlen(pattern)) != 0 ||
	    memcmp(malloced+87*strlen(pattern), pattern, strlen(pattern)) != 0)
		printf("Malloc test: FAILED\n");
	else
		printf("Malloc test: OK\n");

	if (exclusion[0] == 'a')
		printf("Exclusion test: OK\n");
	else
		printf("Exclusion test: FAILED\n");


	close(fd);

	unlink("/tmp/ckpt_test");

	return 0;
}

