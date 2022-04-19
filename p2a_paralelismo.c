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
	int n, count, count_r, count_p, done = 0;
	/*
		count_r -> count del proceso root
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
			MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

			if (n == 0) done = 1;
			else {
				count_r = operacion(numProcs, rank, n); // Proceso 0 tambien opera

				/* Actualizar count */
				MPI_Reduce(&count_r, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

				pi = ((double) count/(double) n)*4.0;
				printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
			}

		} else {	/* Funcion resto de procesos */
			/* usar broadcasat tambien para recibir los datos */
			MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

			if (n == 0) done = 1;
			else {
				/* Calcular nºpuntos y enviarlos a count en el proceso root */
				count_p = operacion(numProcs, rank, n);
				MPI_Reduce(&count_p, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();		// Espera por todos los procesos para terminar
	return 0;
}
