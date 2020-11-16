/*
 * integral_tree_send.c
 *
 *  Created on: 16-Nov-2020
 *      Author: prateek
 *      Pg : 92
 *      Desc : Parallel Trapezoidal Rule; uses a MPI_BCast to calculate integral.
 *      Input :
 *      	a, b : Limits of integration
 *      	n : no of trapezoids
 *      Output :
 *      	Estimate of the integral from a to b of f(x)
 *      	using the trapezoidal rule and n trapezoids
 *
 *      NOTES:
 *      	1. f(x) is hardwired
 *      	2. MPI_Bcast hides the internal implementation of the sending messages to multiple process ( might be implemented
 *      	   using tree like structure that we had implemented manually ).
 *      	4. This version of Get_data() is much more compact and readily comprehensible than both the original and
 *      	   our hand coded tree structure broadcast. Also faster as it is efficiently implemented by MPI_Bast implementation.
 */
#include <stdio.h>
#include <mpi/mpi.h>

float calculate_integral( float local_a, float local_b , float local_n , float h );
float f(float x);
void Get_data2(float *a_ptr, float *b_ptr, int *n_ptr, int my_rank);

int main(int argc, char **argv) {
    int         my_rank;   				/* My process rank           */
    int         no_of_process;			/* The number of processes   */
    float       a;         				/* Left endpoint             */
    float       b;         				/* Right endpoint            */
    int         no_of_trapezoids;       /* Number of trapezoids      */
    float       h;         				/* Trapezoid base length     */
    float       local_a;   				/* Left endpoint my process  */
    float       local_b;   				/* Right endpoint my process */
    int         local_n;   				/* Number of trapezoids for  */
                           	   	   	   	/* my calculation            */
    float       integral;  				/* Integral over my interval */
    float       total;     				/* Total integral            */
    int         source;    				/* Process sending integral  */
    int         dest = 0;  				/* All messages go to 0      */
    int         tag = 0;
    MPI_Status  status;

    /* Let the system do what it needs to start up MPI */
     MPI_Init(&argc, &argv);

	/* Get my process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out how many processes are being used */
	MPI_Comm_size(MPI_COMM_WORLD, &no_of_process);

	/* Send of receive data ( tree hierarchy (height of log(no_of_processes) rather than no_of_processes ) */
	Get_data2(&a, &b, &no_of_trapezoids, my_rank);

	h = (b - a) / no_of_process; 				/* h is the same for all processes */
	local_n = no_of_trapezoids / no_of_process; /* So is the number of trapezoids */

	/* Length of each process' interval of
	 * integration = local_n*h.  So my interval
	 * starts at: */
	local_a = a + my_rank * local_n * h;
	local_b = local_a + local_n * h;
	integral = calculate_integral(local_a, local_b , local_n, h);

	if( my_rank == 0 )
	{
		total = integral;
		for( source = 1 ; source < no_of_process ; source++ )
		{
			MPI_Recv(&integral, 1, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &status);
			total += integral;
		}
	}
	else
	{
		MPI_Send(&integral, 1, MPI_FLOAT, dest, tag, MPI_COMM_WORLD);
	}

	/* Print the result */
	if( my_rank == 0 )
	{
		printf("With no_of_trapezoids = %d , our estimate of integral from %f to %f = %f \n", no_of_process,a,b,total);
	}

	/* Shut down MPI */
	MPI_Finalize();

	return 0;
}

/********************************************************************/
/* Function Get_data2
 * Reads in the user input a, b, and n.
 * Input parameters:
 *     1.  int my_rank			:  rank of current process.
 *     2.  int no_of_process	:  number of processes.
 * Output parameters:
 *     1.  float* a_ptr	:  pointer to left endpoint a.
 *     2.  float* b_ptr	:  pointer to right endpoint b.
 *     3.  int* n_ptr	:  pointer to number of trapezoids.
 * Algorithm:
 *     1.  Process 0 prompts user for input and
 *         reads in the values.
 *     2.  Process 0 sends input values to other
 *         processes using three calls to MPI_Bcast.
 *********************************************************************/
void Get_data2(
        	float*  a_ptr    /* out */,
			float*  b_ptr    /* out */,
			int*    n_ptr    /* out */,
			int     my_rank  /* in  */
			)
{
    if( my_rank == 0 )
    {
    	printf("Enter a , b and n \n");
    	scanf("%f %f %d", a_ptr, b_ptr, n_ptr);
    }

    MPI_Bcast(a_ptr, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(b_ptr, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(n_ptr, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

/* Function to calculate integral */
float calculate_integral( float local_a, float local_b , float local_n , float h )
{
	float integral;		// Store result of integral
	float x ;
	int i;

	integral = ( f(local_a) + f(local_b)) / 2.0;
	x = local_a;

	for( i = 1 ; i <= local_n-1 ; i++ )
	{
		x = x+h;
		integral = integral + f(x);
	}

	integral = integral * h;
	return integral;
}


// f(x) = x^2
float f(float x)
{
	return (x*x);
}








