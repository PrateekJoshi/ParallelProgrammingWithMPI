/*
 * integral_parallel.c
 *
 *  Created on: 16-Nov-2020
 *      Author: prateek
 *      Pg : 78
 *      Desc : Parallel Trapezoidal Rule
 *      Input : None
 *      Output : Estimate of the integral from a to b of f(x) using the trapezoidal rule and n trapezoids
 *      Algorithm:
 *      1) Each process calculates "its" interval of integration
 *      2) Each process estimates the integral of f(x) over its interval using the trapezoidal rule.
 *      3a)Each process != 0 sends its integral to 0.
 *      3b)Process 0 sums the calculations received from the individual processes and prints the results
 *
 *      NOTE : f(x) , a , b and n are all hardwired
 */
#include <stdio.h>

/* We'll be using MPI routines , definitions , etc */
#include <mpi/mpi.h>

float calculate_integral( float local_a, float local_b , float local_n , float h );
float f(float x);

int main(int argc, char **argv)
{
	int my_rank ; 		// My process rank
	int no_of_process;	// No of processes
	float a = 0.0;		// Left endpoint
	float b = 1.0;		// Right endpoint
	int n = 1024;		// No of trapezoids
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

	/* Let the system do waht it needs to start up MPI */
	MPI_Init(&argc, &argv);

	/* Get my process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out how many processes are being used */
	MPI_Comm_size(MPI_COMM_WORLD, &no_of_process);

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






