/*
 * integral_parallel.c
 *
 *  Created on: 16-Nov-2020
 *      Author: prateek
 *      Pg :83
 *      Desc : Parallel Trapezoidal Rule
 *      Input : None
 *      Output : Estimate of the integral from a to b of f(x) using the trapezoidal rule and n trapezoids
 *      Algorithm:
 *      1) Each process calculates "its" interval of integration
 *      2) Each process estimates the integral of f(x) over its interval using the trapezoidal rule.
 *      3a)Each process != 0 sends its integral to 0.
 *      3b)Process 0 sums the calculations received from the individual processes and prints the results
 *
 *      NOTE : f(x) , a , b and n are all read from process 0 and sent to other processes using MPI
 */
#include <stdio.h>

/* We'll be using MPI routines , definitions , etc */
#include <mpi/mpi.h>

float calculate_integral( float local_a, float local_b , float local_n , float h );
float f(float x);

/*
 * Function : Get_data
 * Reads in the user input a, b and n
 * Input params:
 * 	1. int my_rank 			: rank of current process
 * 	2. int no_of_processes	: no of processes
 *
 * 	Output params:
 * 	1. float *a_ptr : pointer to left endpoint a
 * 	2. float *b_ptr : pointer to right endpoint b
 * 	3. int *n_ptr 	: pointer to no of trapezoids
 *
 * 	Algorithm:
 * 	1. Process 0 prompts user for input and reads in values
 * 	2. Process 0 sends input values to other processes.
 */
void Get_data(float *a_ptr, float *b_ptr, int *n_ptr, int my_rank, int no_of_processes)
{
	int source ;	// All local variables used by
	int dest;
	int tag;
	MPI_Status status;

	if( my_rank == 0 )
	{
		printf("Enter a, b and n \n");
		scanf("%s %f %d", a_ptr, b_ptr, n_ptr);
		for( dest = 1 ; dest < no_of_processes ; dest++ )
		{
			tag = 0 ;
			MPI_Send(a_ptr, 1, MPI_FLOAT, dest, tag, MPI_COMM_WORLD);

			tag = 1;
			MPI_Send(b_ptr, 1, MPI_FLOAT, dest, tag, MPI_COMM_WORLD);

			tag = 2;
			MPI_Send(n_ptr, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
		}
	}
	else
	{
		tag = 0 ;
		MPI_Recv(a_ptr, 1, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &status);

		tag = 1;
		MPI_Recv(b_ptr, 1, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &status);

		tag = 2;
		MPI_Recv(n_ptr, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
	}
}


int main(int argc, char **argv)
{
	int my_rank ; 		// My process rank
	int no_of_process;	// No of processes
	float a = 0.0;		// Left endpoint
	float b = 0.0;		// Right endpoint
	int n = 0;			// No of trapezoids
	float h;			// Trapezoids base length
	float local_a;		// Left endpoint my process
	float local_b;		// Right endpoint my process
	int local_n;		// No of trapezoids for my calculation
	float integral;		// Integral over my interval
	float total;		// Total integral
	int source;			// Process sending integral
	int dest = 0 ;		// All messages go to 0
	int tag = 0 ;
	MPI_Status status;

	/* Let the system do what it needs to start up MPI */
	MPI_Init(&argc, &argv);

	/* Get my process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out how many processes are being used */
	MPI_Comm_size(MPI_COMM_WORLD, &no_of_process);

	/* IMPORTANT : Get the input for the process
	 *  Process 0 will read the input from stdin and send it to other processes
	 */
	Get_data(&a, &b, &n, my_rank, no_of_process);

	h = (b-a)/n;					// h is same for all processes
	local_n = n/no_of_process;		// no of trapezoids for individual processes

	/* Length of each process's interval of integration = local_n * h . So my interval start at */
	local_a = a + my_rank * local_n * h;
	local_b = local_a + local_n * h;
	integral = calculate_integral(local_a, local_b , local_n, h);

	/* Add up the integrals calculated by each process */
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
		printf("With n = %d trapezoids , our estimate \n", no_of_process);
		printf("of the integral from %f to %f = %f \n", a, b , total);
	}

	/* Shut down MPI */
	MPI_Finalize();
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






