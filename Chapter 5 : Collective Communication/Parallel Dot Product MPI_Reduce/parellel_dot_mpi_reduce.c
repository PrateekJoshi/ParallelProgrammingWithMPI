/*
 * parellel_dot_mpi_reduce.c
 *
 *  Created on: 21-Nov-2020
 *      Author: prateek
 *  Pg: 98
 *  Desc : compute a dot product of a vector distributed among
 *     the processes.  Uses a block distribution of the vectors.
 *  Input :
 *  	arr_size	: global order of vectors
 *  	x, y		: Vectors
 *
 *  Output:
 *  	The dot product of x & y
 *  Note: Arrays containing vectors are sttaically allocated. Assumes n, the global
 *  order of the vectors is divisible by no_of_process ( no of process)
 */
#include <stdio.h>
#include <mpi/mpi.h>

/* Function decralations */
void Read_vector(char *prompt, 			// in
					float local_v[],	// out
					int local_arr_size,	// in
					int no_of_process,	// in
					int my_rank			// in
					);

float Parallel_dot( float local_x[],	// in
					float local_y[],	// in
					int vector_size);	// in

float serial_dot(float x[],	// in
				 float y[],	// in
				 int n);	// in


#define MAX_LOCAL_ORDER 100

int main(int argc, char **argv)
{
	float local_x[MAX_LOCAL_ORDER];
	float local_y[MAX_LOCAL_ORDER];
	int global_arr_size;
	int local_arr_size;
	float dot;
	int no_of_process;
	int my_rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &no_of_process);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	if( my_rank == 0 )
	{
		printf("Enter the order of the vectors\n");
		scanf("%d", &global_arr_size);
	}

	MPI_Bcast(&global_arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/* Local arr size ( on which a process will calculate dot product : sub problem size) */
	local_arr_size =  global_arr_size / no_of_process;

	/* Read the two vectors on which dot product will be calculated */
	Read_vector("First vector :", local_x, local_arr_size, no_of_process, my_rank);
	Read_vector("Second vector :", local_y, local_arr_size, no_of_process, my_rank);

	/* Now we have two vectors on which dot product is to be calculated,
	 * Now calculate dot product parallel
	 */
	dot = Parallel_dot(local_x, local_y, local_arr_size);

	/* Print the result if its root process */
	if( my_rank == 0 )
	{
		printf("The dot product is %f \n", dot);
	}

	MPI_Finalize();

	return 0;
}

/*
 * Function to read a vector in local_v of size local_arr_size
 */
void Read_vector(char *prompt, 			// in
					float local_v[],	// out
					int local_arr_size,	// in
					int no_of_process,	// in
					int my_rank			// in
					)
{
	int i, process;
	float temp_arr[MAX_LOCAL_ORDER] = {0} ;	// arr that will be used to take user input
	MPI_Status status;

	/* If root process */
	if( my_rank == 0 )
	{
		/* Fill local_v[] for process 0 as other process will receive it. For root process
		 * user will provide the input
		 */
		printf("Enter %s \n", prompt);
		for( i = 0 ; i < local_arr_size ; i++ )
		{
			scanf("%f", &local_v[i]);
		}

		/* For each process, take input vector elements. These vector will be sent to corresponding
		 * process to calculate dot product
		 */
		for( process = 1 ; process < no_of_process ; process++ )
		{
			for( i = 0 ; i < local_arr_size ; i++ )
			{
				scanf("%f", &temp_arr[i]);
			}

			/* Now send the temp arr to the process as input */
			MPI_Send(temp_arr, local_arr_size, MPI_FLOAT, process, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		/* If not root process, then receive local vector ( subprob) on which dot product is to be calculated */
		MPI_Recv(local_v, local_arr_size, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
	}
}

/*
 * Function to calculate dot product of two vactors parallelly
 */
float Parallel_dot( float local_x[],	// in
					float local_y[],	// in
					int vector_size)	// in
{
	float local_dot;
	float dot = 0.0;

	/* Calculate dot product of the subproblem */
	local_dot = serial_dot(local_x, local_y, vector_size);

	/* Use MPI_reduce to accumulate subproblem result to global solution */
	MPI_Reduce(&local_dot, &dot, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);


	return dot;
}

/*
 * Function to calculate dot product serially
 */
float serial_dot(float x[],	// in
				 float y[],	// in
				 int n)		// in
{
	int i;
	float sum = 0.0;

	for( i = 0 ; i < n ; i++ )
	{
		sum += x[i]*y[i];
	}

	return sum;
}


