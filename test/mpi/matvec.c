#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <mpi.h>

#include "../../src/checkpoint.h"
#include "../../automatic/online/online.h"


int main ( int argc, char *argv[] )

//********************************************************************
//
//  Purpose:
//
//    MATVEC uses MPI to compute a matrix-vector product c = A * b.
//
//  Discussion:
//
//    This is the simple self-scheduling version.  Each worker is given 
//    a copy of b, and then is fed one row of A.  As soon as it computes 
//    C(I) = A(I,1:N)*b(1:N), it is given another column of A, unless
//    there are no more, in which case it is sent a "terminate" message. 
//    Thus, a faster process will be given more work to do.
//
//    By using allocatable arrays, the amount of memory used has been
//    controlled.  The master process allocates A and b, but the worker
//    processes only allocate enough memory for one row of A, and b.
//
//  Modified:
//
//    01 May 2003
//
//  Author:
//
//    John Burkardt
//
//  Reference:
//
//    Gropp, Lusk, Skjellum,
//    Using MPI,
//    Portable Parallel Programming with the Message-Passing Interface,
//    MIT Press, 1997.
//
//    Snir, Otto, Huss-Lederman, Walker, Dongarra,
//    MPI - The Complete Reference,
//    Volume 1, The MPI Core,
//    second edition,
//    MIT Press, 1998.
//
//    Translated to C, Andrey Smirnov.
//
{
  double *a;
  double *a_row;
  double ans;
  double *b;
  double *c;
  int dest;
  int dummy;
  int i;
  int j;
  int j_one;
  int k;
  int m;
  int master = 0;
  int my_id;
  int n;
  int num_procs;
  int num_rows;
  int num_workers;
  double pi = 3.14159265358979323846264338327950288419716939937510;
  MPI_Status status;
  int tag;
  int tag_done = 10000;
//
//  Initialize MPI.
//
  MPI_Init ( &argc, &argv );
//
//  Get this processor's ID.
//
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
//
//  Get the number of processors.
//
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  if ( my_id == 0 ) 
  {
    printf("\n");
    printf("MATVEC - Master process:\n");
    printf("  A C program, using MPI to compute\n");
    printf("  a matrix-vector product c = A * b.\n");
    printf("\n");
    printf("  Compiled on %s at %s\n", __DATE__, __TIME__);
    printf("\n");
    printf("  The number of processes is %d.\n", num_procs);
  }
  printf("\n");
  printf("Process %d is active.\n", my_id);
//
//  Let the size of the array be determined by process 0.
//  The other processes need to know N, but not M!
//
  if ( my_id == 0 ) 
  {
    m = 100;
    n = 50;
    printf("\n");
    printf("  The number of rows is %d\n", m);
    printf("  The number of columns is %d\n", n);
  }
  MPI_Bcast(&n, 1, MPI_INT, master, MPI_COMM_WORLD);
//
//  The master process allocates and initializes A and B.
//
//  Because we are dynamically allocating A, we can't use 2D array double
//  indexing, so we have to figure out where we are on our own.
//
  if ( my_id == master )
  {
    a = (double *)malloc(sizeof(double)*m*n);
    b = (double *)malloc(sizeof(double)*n);
    c = (double *)malloc(sizeof(double)*m);

    k = 0;
    for ( i = 1; i <= m; i++ ) 
    {
      for ( j = 1; j <= n; j++ )
      {
        a[k] = sqrt ( 2.0E+00 / ( double ) ( n + 1 ) ) 
          * sin ( ( double ) ( i * j ) * pi / ( double ) ( n + 1 ) );
        k = k + 1;
      }
    }
//
//  B is specially chosen so that C = A * B is known in advance.
//  The value of C will be zero, except that entry J_ONE will be 1.
//  Pick any value of J_ONE between 1 and M.
//
    j_one = 17;
    for ( i = 0; i < n; i++ )
    {
      b[i] = sqrt ( 2.0E+00 / ( double ) ( n + 1 ) ) 
        * sin ( ( double ) ( ( i + 1 ) * j_one ) * pi / ( double ) ( n + 1 ) );
    }

    printf("\n");
    printf("MATVEC - Master process:\n");
    printf("  Right hand side b\n");
    printf("\n");
    for ( i = 0; i < n; i++ )
	    printf("%6d %10f\n", i, b[i]);
  }
//
//  Worker processes set aside room for one row of A, and for the 
//  vector B.
//
  else
  {
    a_row = (double *)malloc(sizeof(double) *n);
    b = (double *)malloc(sizeof(double) *n);
  }
//
//  Process 0 broadcasts the vector B to the other processes.
//
  MPI_Bcast(b, n, MPI_DOUBLE, master, MPI_COMM_WORLD);

  if ( my_id == master )
//
//  Process 0 sends one row of A to all the other processes.
//
//  If we were using standard C 2D array storage, the entries of
//  the row would be contiguous; using pointers, we have ended up
//  in the same situation.  As long as the entries are contiguous,
//  we can use a simple standard datatype with MPI_Send.  
//
//  The situation would require a little more work if we tried
//  to send a column of data instead of a row.
//
  {
    num_rows = 0;

    for ( i = 1; i <= num_procs-1; i++ )
    {
      dest = i;
      tag = num_rows;
      k = num_rows * n;

      MPI_Send(a+k, n, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);

      num_rows = num_rows + 1;
    }
     
    num_workers = num_procs-1;

    for ( ; ; )
    {
      MPI_Recv( &ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      tag = status.MPI_TAG;
      c[tag] = ans;

      if ( num_rows < m )
      {
        num_rows = num_rows + 1;
        dest = status.MPI_SOURCE;
        tag = num_rows;
        k = num_rows * n;

        MPI_Send( a+k, n, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD );
      }
      else
      {
        num_workers = num_workers - 1;
        dummy = 0;
        dest = status.MPI_SOURCE;
        tag = tag_done;

        MPI_Send ( &dummy, 1, MPI_INT, dest, tag, MPI_COMM_WORLD );

        if ( num_workers == 0 )
        {
          break;
        }
      }

    }

    free(a);
    free(b);
  }
//
//  Each worker process repeatedly receives rows of A (with TAG indicating
//  which row it is), computes dot products A(I,1:N) * B(1:N) and returns
//  the result (and TAG), until receiving the "DONE" message.
//
  else
  {
    for ( ; ; )
    {
      MPI_Recv ( a_row, n, MPI_DOUBLE, master, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

      tag = status.MPI_TAG;

      if ( tag == tag_done ) 
      {
        break;
      }

      ans = 0.0E+00;
      for ( i = 0; i < n; i++ )
      {
        ans = ans + a_row[i] * b[i];
      }

      MPI_Send ( &ans, 1, MPI_DOUBLE, master, tag, MPI_COMM_WORLD );

    }

    free(a_row);
    free(b);
  }
//
//  Print out the answer.
//
  if ( my_id == master ) 
  {
    printf("\n");
    printf("MATVEC - Master process:\n");
    printf("  Product vector c = A * b\n");
    printf("  (Should be zero, except for a 1 in entry %d\n", j_one-1);
    printf("\n");
    for ( i = 0; i < m; i++ )
	    printf("%4d %10f\n", i, c[i]);
   free(c);
  }
//
//  End of execution.
//
  MPI_Finalize ( );

  if ( my_id == master ) 
  {
    printf("\n");
    printf("MATVEC - Master process:\n");
    printf("  Normal end of execution.\n");
  }

  return 0;
}




