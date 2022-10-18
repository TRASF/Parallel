#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>

double startTime, stopTime;

void *merge(int *A, int sizeA, int *B, int sizeB)
{

    int ai, bi, ci, i;
    int *dummy;
    int size = sizeA + sizeB;
    ai = 0;
    bi = 0;
    ci = 0;
    dummy = malloc(size * sizeof(int)); /*Statical alllcated for the array*/

    while ((ai < sizeA) && (bi < sizeB)) /* Merge A and B elements in to C [ Sorted, Normal case ]*/
    {
        if (A[ai] <= B[bi])
        {
            dummy[ci] = A[ai];
            ci++;
            ai++;
        }
        else
        {
            dummy[ci] = B[bi];
            ci++;
            bi++;
        }
    }
    /* Shorter case put in the remaining elements [ start current index of c | end N]*/
    if (ai >= sizeA) /*A is shorter*/
        for (i = ci; i < size; i++, bi++)
            dummy[i] = B[bi];
    else if (bi >= sizeB) /*B is shorter*/
        for (i = ci; i < size; i++, ai++)
            dummy[i] = A[ai];

    for (i = 0; i < sizeA; i++) /* Put the elements back to A [0 - size of A] */
        A[i] = dummy[i];
    for (i = 0; i < sizeB; i++) /* Put the elements back to B [ size of A + i - size of B] */
        B[i] = dummy[sizeA + i];
}

void mergeSort(int *A, int left, int right)
{
    if (left == right) /* min > max discard*/
        return;
    int *dummy;
    int mid = (left + right) / 2;
    int lower = mid - left + 1;
    int upper = right - mid;

    mergeSort(A, left, mid);      /* Sort for the first half */
    mergeSort(A, mid + 1, right); /* Sort for the second half */
    dummy = merge(A + left, lower, A + mid + 1, upper);
}

int main(int argc, char *argv[])
{
    int rankWorld, sizeWorld;
    int globalSize, localSize, i;
    int *global, *local;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankWorld);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeWorld);

    globalSize = atoi(argv[1]); /* Get input from mpirun -np 4 ./sort 10000 < from here */

    global = (int *)malloc(globalSize * sizeof(int)); /* Unsorted array for initial process */

    for (i = 0; i < globalSize; i++) /* Put a random number in range of 0 - 9999 */
        global[i] = (rand() %
                     (9999 - 0 + 1)) +
                    0;

    // for (i = 0; i < 5; i++)
    //     printf("%d ", global[i]); /* Print to test array sorted and random number is valid */
    // printf("\n");
    localSize = globalSize / sizeWorld;             /* Initiate a local length for local array */
    local = (int *)malloc(localSize * sizeof(int)); /* For sorted [ Master and Worker will work with this ]*/
    MPI_Scatter(global, localSize, MPI_INT, local, localSize, MPI_INT, 0, MPI_COMM_WORLD);
    srand(time(0)); /* for timer */

    startTime = MPI_Wtime(); /* Start counting a runtime */
    int sendCount;
    sendCount = globalSize / sizeWorld; /* Find the time that have to send */
    if (rankWorld == 0)                 /* Master task */
    {
        srandom(MPI_Wtime());
        MPI_Scatter(global, sendCount, MPI_INT, local, sendCount, MPI_INT, 0, MPI_COMM_WORLD);
        mergeSort(local, 0, sendCount - 1);
    }
    else /* Worker task */
    {
        MPI_Scatter(global, sendCount, MPI_INT, local, sendCount, MPI_INT, 0, MPI_COMM_WORLD);
        mergeSort(local, 0, sendCount - 1);
    }
    // for (i = 0; i < 5; i++)
    //     printf("%d ", local[i]);
    // printf("\n");
    MPI_Barrier(MPI_COMM_WORLD);
    stopTime = MPI_Wtime();
    if (rankWorld == 0)
    {
        printf("Start: %f End: %f Time consumed: %f\n", startTime, stopTime, stopTime - startTime);
        free(global);
    }

    free(local);
    MPI_Finalize();
    return 0;
}