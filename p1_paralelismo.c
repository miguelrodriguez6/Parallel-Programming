#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int operacion(int numProcs, int rank, int n) {
	int i, count = 0;
	double x, y, z;

	for (i = rank+1; i <= n; i+=numProcs) {
		// Get the random numbers between 0 and 1
		x = ((double) rand()) / ((double) RAND_MAX);
		y = ((double) rand()) / ((double) RAND_MAX);

		// Calculate the square root of the squares
		z = sqrt((x*x)+(y*y));

		// Check whether z is within the circle
		if(z <= 1.0)
			count++;
	}
	return count;
}

int main(int argc, char *argv[])
{
	int numProcs, rank;
	int n, count, send_p, recv_p, count_p, done = 0;
	/*
		send_p	-> bucle enviar a todos los procesos
		recv_p	-> bucle recibir de todos los procesos
		count_p	-> count de cada proceso individual
	*/
    double PI25DT = 3.141592653589793238462643;
    double pi;

	MPI_Init(&argc, &argv);						// inicializar MPI
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);	// numero de procesos totales
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);		// numero del proceso

	while(!done) {

		count = 0;

		if (rank == 0) {	/* Funcion proceso 0 */
			printf("Enter the number of points: (0 quits) \n");
			scanf("%d",&n);

			/* Enviar n a los procesos */
			for(send_p = 1; send_p < numProcs ; send_p++)
				MPI_Send(&n, 1, MPI_INT, send_p, 1, MPI_COMM_WORLD);	// Tag 1 (aleatorio)

			if (n == 0) done = 1;
			else {
				count = operacion(numProcs, rank, n); // Proceso 0 tambien opera

				/* Recibir mensajes de los otros procesos y actualizar count */
				for (recv_p = 1; recv_p < numProcs; recv_p++) {
					int aux;
					MPI_Recv(&aux, 1, MPI_INT, recv_p, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					count += aux;
				}

				pi = ((double) count/(double) n)*4.0;
				printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
			}

		} else {	/* Funcion resto de procesos */
			/* Recibir n del proceso 0 */
			MPI_Recv(&n, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			if (n == 0) done = 1;
			else {
				/* Calcular nºpuntos y enviarlos al proceso 0 */
				count_p = operacion(numProcs, rank, n);
				MPI_Send(&count_p, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);	// Tag 2 (aleatorio)
			}
		}
	}

	MPI_Finalize();		// Espera por todos los procesos para terminar
	return 0;
}
