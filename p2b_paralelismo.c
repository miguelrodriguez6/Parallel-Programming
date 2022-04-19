#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/* Solo para MPI_SUM */
int MPI_FlattreeColectiva(void *buff, void *recvbuff, int count,
	MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {

		if (count < 0) return MPI_ERR_COUNT;
		if (buff == recvbuff || buff == NULL) return MPI_ERR_BUFFER;

		int i, proc, rank, numProcs;
		MPI_Comm_size(comm, &numProcs);
		MPI_Comm_rank(comm, &rank);

		if (rank == root) {	/* si es root opera */
			int errorCheck;
			int tmp[count];

			/* Añadir datos de root a recvbuff */
			for (i = 0; i < count; i++)
				((int*)recvbuff)[i] = ((int*)buff)[i];

			/* Añadir datos de cada proceso a root */
			for (proc = 0; proc < numProcs; proc++) {
				if (proc != rank) {	// si no es root recibe datos del proceso proc

					errorCheck = MPI_Recv(&tmp, count, datatype, proc, 0, comm, MPI_STATUS_IGNORE);	// tag aleatorio 0
					if (errorCheck != MPI_SUCCESS) return errorCheck;

					for (i = 0; i < count; i++)
						((int*)recvbuff)[i] = ((int*)recvbuff)[i] + tmp[i];
				}
			}
			return MPI_SUCCESS;

		} else	/* si no es root envia datos a root */
			MPI_Send(buff, count, datatype, root, 0, comm);	// tag aleatorio 0
}

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

			if (n == 0) done = 1; //finalizar programa
			else {
				count_r = operacion(numProcs, rank, n); // Proceso 0 tambien opera

				/* Actualizar count */
				MPI_FlattreeColectiva(&count_r, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

				pi = ((double) count/(double) n)*4.0;
				printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
			}

		} else {	/* Funcion resto de procesos */
			/* usar broadcasat tambien para recibir los datos */
			MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

			if (n == 0) done = 1;	// finalizar programa
			else {
				/* Calcular nºpuntos y enviarlos a count en el proceso root */
				count_p = operacion(numProcs, rank, n);
				MPI_FlattreeColectiva(&count_p, &count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();		// Espera por todos los procesos para terminar
	return 0;
}
