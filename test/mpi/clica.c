/*
 *
 * Test for libcheckpoint 
 *
 */


#include <mpi.h>
#include <stdio.h>

#include "../../src/checkpoint.h"
#include "../../automatic/online/online.h"

int n, me;

void send_turn()
{
	int i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
		{
			int data;
			
			if (me == 2 && (i*n + j) % 4 ==0)
			   ckptMPIOnlineCheckpoint();

			if (i == j || (i != me && j != me))
				continue;
				
				
			fprintf(stderr, "I'm %d, doing %d --> %d\n", me, i, j);
			if (me == i)
			{
				data = (i+1)*(j+1);
				fprintf(stderr, "I'm %d, sending %d\n", me, data);
				MPI_Send(&data, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
				fprintf(stderr, "I'm %d, sent\n", me, data);
			}
			else if (me == j)
			{
				MPI_Status status;
				fprintf(stderr, "I'm %d, receiving\n", me, data);
				MPI_Recv(&data, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
				fprintf(stderr, "I'm %d, received %d\n", me, data);
				if (data != (i+1)*(j+1))
				    abort();
			}
		}
}

void ckptBenchmarkPrint();

int main(int argc, char *argv[])
{
	int i;
        char buf[256];

	MPI_Init(&argc, &argv);

        MPI_Comm_size(MPI_COMM_WORLD, &n);                         
        MPI_Comm_rank(MPI_COMM_WORLD, &me);                           

        sprintf(buf, "clica.%d.log", me);
        freopen(buf, "w", stderr);
        setbuf(stderr, NULL);

	/* do a turn of communication */
	for (i = 0; i < 6; i++)
        {
                fprintf(stderr, "\n\n=== TURN %d\n", i);
		send_turn();
        }
		
	/*if (me == 1)
    	    ckptMPIOnlineCheckpoint();*/
	/*ckptCheckpoint();
	ckptBenchmarkPrint();*/
	
	/* do a turn of communication */
	/*for (i = 0; i < 3; i++)
		send_turn();*/

	MPI_Finalize();
	
	return 0;
}

