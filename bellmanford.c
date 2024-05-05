#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define INF 1000000000

int readFile(const char* filename, int* n, int* m, int** edges) {
    FILE* file = fopen(filename , "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return -1;
    }

    fscanf(file, "%d %d", n, m);
    *edges = (int*)malloc(*m * 3 * sizeof(int));

    for (int i = 0; i < *m; i++) {
        fscanf(file, "%d %d %d", *edges + (i * 3), *edges + (i * 3 + 1), *edges + (i * 3 + 2));
    }

    fclose(file);

    return 0;
}

int main(int argc, char** argv) {
    // MPI Initialization
    MPI_Comm world;
    int numProc;
    int myRank;

    MPI_Init( &argc, &argv );
    world  = MPI_COMM_WORLD;
    MPI_Comm_size( world, &numProc);
    MPI_Comm_rank( world, &myRank);

    // Read input file
    int numV, numE;
    int* edges;

    if (myRank == 0) {
        if (argc <= 1) {
            fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
            return -1;
        }

        if (readFile(argv[1], &numV, &numE, &edges) != 0) {
            return -1;
        }
    }

    // Start timer
    MPI_Barrier(world);
    double timer_start, timer_end;
    if (myRank == 0) {
        timer_start = MPI_Wtime();
    }

    // Bellman-Ford Algorithm

    // Broadcast Number of Vertices and Edges
    MPI_Bcast(&numV, 1, MPI_INT, 0, world);
    MPI_Bcast(&numE, 1, MPI_INT, 0, world);

    // Broadcast edges
    if (myRank != 0) {
        edges = (int*)malloc(numE * 3 * sizeof(int));
    }

    MPI_Bcast(edges, numE * 3 * sizeof(int), MPI_BYTE, 0, world);

    // Calculate local edges
    int edgesPerProcess = numE / numProc;
    int rem = numE % numProc;
    int startEdge = myRank * edgesPerProcess;
    int endEdge = startEdge + edgesPerProcess;
    if (myRank == numProc - 1) {
        endEdge += rem;
    }    

    // Initialize distance array
    int* dist = (int*)malloc(numV * sizeof(int));
    int* localDist = (int*)malloc(numV * sizeof(int));
    if(myRank == 0) {
        for (int i = 0; i < numV; i++) {
            dist[i] = INF;
        }
        dist[0] = 0;
    }

    // Relax edges
    for (int i = 0; i < numV - 1; i++) {
        // Divide work
        MPI_Bcast(dist, numV, MPI_INT, 0, world);
        for (int i = 0; i < numV; i++) {
            localDist[i] = dist[i];
        }
        for (int j = startEdge; j < endEdge; j++) {
            int u = edges[j * 3];
            int v = edges[j * 3 + 1];
            int w = edges[j * 3 + 2];

            if (localDist[u] + w < localDist[v]) {
                localDist[v] = localDist[u] + w;
            }
        }
        // Gather results
        MPI_Reduce(localDist, dist, numV, MPI_INT, MPI_MIN, 0, world);
    }

    // Check negative cycle
    int hasNegativeCycle = 0;
    MPI_Bcast(dist, numV, MPI_INT, 0, world);

    for (int i = startEdge; i < endEdge; i++) {
        int u = edges[i * 3];
        int v = edges[i * 3 + 1];
        int w = edges[i * 3 + 2];

        if (dist[u] + w < dist[v]) {
            hasNegativeCycle = 1;
            break;
        }
    }

    MPI_Allreduce(MPI_IN_PLACE, &hasNegativeCycle, 1, MPI_INT, MPI_MAX, world);
    MPI_Barrier(world);

    //Show Results
    if (myRank == 0) {
        // End timer
        timer_end = MPI_Wtime();

        // Print results
        if (hasNegativeCycle) {
            printf("Graph contains negative cycle\n");
        } else {
            for (int i = 0; i < numV; i++) {
                printf("Distance from 0 to %d: %d\n", i, dist[i]);
            }
        }

        printf("Time: %f\n", timer_end - timer_start);
    }

    // Clean up
    free(edges);
    MPI_Finalize();
    return 0;
}
