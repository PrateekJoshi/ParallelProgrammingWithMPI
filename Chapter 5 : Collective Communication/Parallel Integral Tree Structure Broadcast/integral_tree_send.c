/*
 * integral_tree_send.c
 *
 *  Created on: 16-Nov-2020
 *      Author: prateek
 *      Pg : 89
 *      Desc : Parallel Trapezoidal Rule; uses a hand-coded tree-structured broadcast.
 *      Input :
 *      	a, b : Limits of integration
 *      	n : no of trapezoids
 *      Output :
 *      	Estimate of the integral from a to b of f(x)
 *      	using the trapezoidal rule and n trapezoids
 *
 *      NOTES:
 *      	1. f(x) is hardwired
 *      	2. The no of processes (no_of_processes) should evenly divide the no of trapezoids(no_of_trapezoids)
 *
 *      									Rank 0							STAGE 0
 *      									/      \
 *									  Rank 0	    Rank 1					STAGE 1
 *									/     \        /       \
 *							  Rank 0    Rank 2    Rank 1    Rank 3			STAGE 2
 */
#include <stdio.h>
#include <mpi/mpi.h>

float calculate_integral( float local_a, float local_b , float local_n , float h );
float f(float x);
int ceiling_log2(int x );

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

    void Get_data1(float *a_ptr, float *b_ptr, int *n_ptr, int my_rank, int no_of_processes);

    /* Let the system do what it needs to start up MPI */
     MPI_Init(&argc, &argv);

	/* Get my process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out how many processes are being used */
	MPI_Comm_size(MPI_COMM_WORLD, &no_of_process);

	/* Send of receive data ( tree hierarchy (height of log(no_of_processes) rather than no_of_processes ) */
	Get_data1(&a, &b, &no_of_trapezoids, my_rank, no_of_process);

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
/* Ceiling of log_2(x) is just the number of times
 * times x-1 can be divided by 2 until the quotient
 * is 0.  Dividing by 2 is the same as right shift.
 ********************************************************************/
int ceiling_log2(int x )
{
	/* Use unsigned so that right shift will fill leftmost bit with 0 */
	unsigned temp = (unsigned) x - 1;
	int result = 0 ;

	while( temp != 0 )
	{
		temp = temp >> 1 ;			// divide by 2
		result += 1;
	}

	return result;
}

/********************************************************************************/
/* I_receive() function returns 1 if during the current stage the calling
 * process receives data. Otherwise returns 0.
 * If the calling process receives data , the parameter source is used to return
 * the rank of sender.
 ********************************************************************************/
int I_receive(int stage 		/* in */,
				int my_rank	 	/* in */,
				int *source_ptr /* out */
		)
{
	// 2^stage = 1 <<stage
	int power_2_stage = 1 << stage;
	if( (power_2_stage <= my_rank) &&  ( my_rank < 2 * power_2_stage) )
	{
		*source_ptr = my_rank - power_2_stage;
		return 1;
	}
	else
	{
		return 0;
	}
}

/********************************************************************************/
/* I_send() function returns 1 if during the current stage the calling
 * process send data. Otherwise returns 0.
 * If the calling process send data , the parameter dest is used to return
 * the rank of receiver.
 ********************************************************************************/
int I_send(	int stage,			/* in */
			int my_rank,		/* in */
			int no_of_process,	/* in */
			int *dest_ptr		/* out */
			)
{
	// 2^stage = 1 << stage
	int power_2_stage = 1 << stage;
	if( my_rank < power_2_stage )
	{
		*dest_ptr = my_rank + power_2_stage;
		/* Check of destination is not more than no of process */
		if( *dest_ptr >= no_of_process )
		{
			return 0;
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 * Send a, b, n to dest process
 */
void Send(	float a,	/* in */
			float b,	/* in */
			float n,	/* in */
			int dest	/* in */
		)
{
	MPI_Send(&a, 1, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
	MPI_Send(&b, 1, MPI_FLOAT, dest, 1, MPI_COMM_WORLD);
	MPI_Send(&n, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
}

/*
 * Receive a_ptr, b_ptr, n_ptr from source process
 */
void Receive(	float *a_ptr,		/* out */
				float *b_ptr,		/* out */
				int *n_ptr,			/* out */
				int source			/* in */
			)
{
	MPI_Status status;

	MPI_Recv(a_ptr, 1, MPI_FLOAT, source, 0, MPI_COMM_WORLD, &status);
	MPI_Recv(b_ptr, 1, MPI_FLOAT, source, 1, MPI_COMM_WORLD, &status);
	MPI_Recv(n_ptr, 1, MPI_FLOAT, source, 2, MPI_COMM_WORLD, &status);
}


/********************************************************************/
/* Function Get_data1
 * Reads in the user input a, b, and n.
 * Input parameters:
 *     1.  int my_rank		:  rank of current process.
 *     2.  int no_of_process:  number of processes.
 * Output parameters:
 *     1.  float* a_ptr	:  pointer to left endpoint a.
 *     2.  float* b_ptr	:  pointer to right endpoint b.
 *     3.  int* n_ptr	:  pointer to number of trapezoids.
 * Algorithm:
 *     1.  Process 0 prompts user for input and
 *         reads in the values.
 *     2.  Process 0 sends input values to other
 *         processes using hand-coded tree-structured
 *         broadcast.
 **********************************************************************/
void Get_data1(
			float *a_ptr,		/* out */
			float *b_ptr,		/* out */
			int *n_ptr,			/* out */
			int my_rank,		/* in */
			int no_of_process	/* in */
			)
{
    int source;
    int dest;

	if( my_rank == 0 )
	{
		printf("Enter a , b and n \n");
		scanf("%f %f %d", a_ptr, b_ptr, n_ptr);
	}

	for( int stage = 0 ; stage < ceiling_log2(no_of_process) ; stage ++)
	{
		if( I_receive(stage, my_rank, &source) )
		{
			Receive(a_ptr, b_ptr, n_ptr, source);
		}
		else if( I_send(stage, my_rank, no_of_process, &dest) )
		{
			Send(*a_ptr, *b_ptr, *n_ptr, dest);
		}
	}
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








