#define RANGE 10000

#include <stdio.h>
#include <stdlib.h> /* for random function */
#include "mpi.h"

void sequentialSort(int *A, int size)
{
    int i, j;
    int temp;
    for (i = 0; i < size; ++i)
    {

        for (j = i + 1; j < size; ++j)
        {

            if (A[i] > A[j])
            {

                temp = A[i];
                A[i] = A[j];
                A[j] = temp;
            }
        }
        // printf("%d ", A[i]);
    }
}

int main(int argc, char *argv[])
{
    double startTime, stopTime;
    int rankWorld, sizeWorld;
    int *global, *local;
    int globalSize, globalBar;
    int i, bucket;
    MPI_Status status;
    MPI_Request sendRequest, recvRequest;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeWorld);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankWorld);

    if (rankWorld == 0)
    {
        globalSize = atoi(argv[1]);

        startTime = MPI_Wtime();
        global = malloc(globalSize * sizeof(int));

        for (i = 0; i < globalSize; i++)
            global[i] = rand() % RANGE - 1;

        printf("\nTotal elements = %d;\nEach process has %d elements.\n", globalSize, globalSize / sizeWorld);
    }
    MPI_Bcast(&globalSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    globalBar = globalSize / sizeWorld;

    local = malloc(globalBar * sizeof(int));
    if (sizeWorld > 1)
    {
        if (rankWorld == 0)
        {
            for (i = 0; i < globalSize; i++)
            {
                bucket = global[i] / (RANGE / sizeWorld);
                if (bucket == sizeWorld)
                    bucket = sizeWorld - 1;
                // printf("Number %d assign to Bucket: %d\n", global[i], bucket);
                MPI_Isend(&global[i], 1, MPI_INT, bucket, 0, MPI_COMM_WORLD, &sendRequest);
            }
        }

        for (i = 0; i < globalBar; i++)
        {
            MPI_Irecv(&local[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &recvRequest);
            // MPI_Wait(&recvRequest, &status);
            // printf("Rank: %d process: %d\n", rankWorld, local[i]);
        }
        // MPI_Barrier(MPI_COMM_WORLD);
        sequentialSort(local, globalBar);
        // MPI_Barrier(MPI_COMM_WORLD);
    }
    else
    {

        sequentialSort(global, globalSize);
    }

    MPI_Gather(local, globalBar, MPI_INT, global, globalBar, MPI_INT, 0, MPI_COMM_WORLD);
    stopTime = MPI_Wtime();

    if (rankWorld == 0)
        printf("\nTime consumed using %d processes: %f Secs\n", sizeWorld, (stopTime - startTime));

    // free(local);

    // if (rankWorld == 0)
    // 	free(global);

    MPI_Finalize();
}
