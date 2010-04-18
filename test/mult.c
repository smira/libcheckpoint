#include <stdio.h>
#include <stdlib.h>

//#define  N        2000
#define  MAXSIZE  10000

static int N = 1000;
static int niter = -1;

void read_matrix(/* int * */);
void mult_matrix(/* int * */);

ckpt_main(argc, argv)
int argc;
char **argv;
{

  int i;
  int *A;
  int *B;
  int num;
  int size;

  if (argc >= 2) 
	  N = atoi(argv[1]);
  if (argc == 3)
	  niter = atoi(argv[2]);
  else
	  niter = (N / 100) - 1;

  size = N * N;

  A = (int *)malloc(sizeof(int) * size);
  B = (int *)malloc(sizeof(int) * size);

  if (!A || !B) {
    printf("not enough memory\n");
    exit(-1);
  }

  read_matrix(A);

  mult_matrix(A, B);

  free(A);
  free(B);

}

void
read_matrix(A)
int A[][N];
{

  int i,j;

  srand(1);

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      A[i][j] = rand() % MAXSIZE;

}

void
mult_matrix(A, B)
int A[][N];
int B[][N];
{

  int i,j,k;
  int size;

  size = N * N;

  printf("initializing matrix B\n");

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      B[i][j] = 0;

  printf("doing the multiplication B = A * A\n");

  for(i=0;i<N;i++) {
    for(j=0;j<N;j++)
      for(k=0;k<N;k++)
	B[i][j] += A[i][k] * A[k][j];
    if ( i % 100 == 0 ) {
      printf("row %d\n",i);
      ckptCheckpoint();
      if (i/100 == niter)
      {
      	ckptBenchmarkPrint();
	exit(0);
      }
    }
  }

  printf("moving the result from matrix B to matrix A\n");

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      A[i][j] = B[i][j];

}
