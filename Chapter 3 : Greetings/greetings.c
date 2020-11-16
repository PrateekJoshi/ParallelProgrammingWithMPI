/*
 * hello.c
 *
 *  Created on: 15-Nov-2020
 *      Author: prateek
 */
#include <stdio.h>
#include <string.h>
#include <mpi/mpi.h>

/*
 * Pg : 63
 * Multiple process p0,p1, p2,.. pn equal to the no of processors available , send the greeting message to process p0.
 * p0 process receives those messages and prints to stdout.
 */

int main(int argc, char **argv) {
	int my_rank;				// rank of process
	int no_of_processes;		//	no of processes
	int source;					// rank of sender
	int dest = 0;				// rank of receiver
	int tag = 0 ;				// tag for messages
	char message[100] = {0};	// Storage buffer for message
	MPI_Status status;			// Return status for receive

	/* Start using MPI */
	MPI_Init(&argc, &argv);

	/* Find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out the no of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &no_of_processes);

	if( my_rank != 0 )
	{
		/* Create message */
		sprintf(message, "Greetings from process %d \n", my_rank);

		/* Use strlen+1 so that '\0' gets transmitted */
		MPI_Send(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
	}
	else
	{
		/* my_rank == 0
		 	 Receive message from all other parallel running processes
		 */
		for( source = 1 ; source < no_of_processes ; source++ )
		{
			MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
			printf("%s\n", message);
		}
	}

	/* Shut down MPI */
	MPI_Finalize();

	return 0;
}





