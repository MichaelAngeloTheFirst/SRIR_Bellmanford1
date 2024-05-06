#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define INF 1000000000

// Funkcja sczytuje krawędzie z pliku i zapisuje ilość wierzchołków, krawędzi oraz krawędzie do zmiennych przekazanych jako argumenty
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
    // Inicjalizacja MPI
    MPI_Comm world;
    int numProc;
    int myRank;

    MPI_Init( &argc, &argv );
    world  = MPI_COMM_WORLD;
    MPI_Comm_size( world, &numProc);
    MPI_Comm_rank( world, &myRank);

    // Sczytanie danych z pliku przez proces master
    int numV, numE;
    int* edges;
    int source = 0;

    if (myRank == 0) {
        if (argc <= 1) {
            fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
            return -1;
        }

        if (readFile(argv[1], &numV, &numE, &edges) != 0) {
            return -1;
        }

        // Pobranie wierzchołka źródłowego z argumentów
        if (argc == 3){
            source = atoi(argv[2]);
        }
    }

    MPI_Barrier(world);

    // Rozpoczęcie pomiaru czasu
    double timer_start, timer_end;
    if (myRank == 0) {
        timer_start = MPI_Wtime();
    }

    // Rozproszony algorytm Bellmana Forda

    // Przekazanie ilości wierzchołków i krawędzi wszystkim procesom.
    MPI_Bcast(&numV, 1, MPI_INT, 0, world);
    MPI_Bcast(&numE, 1, MPI_INT, 0, world);
    MPI_Bcast(&source, 1, MPI_INT, 0, world);

    // Alokacja pamięci na krawędzie dla procesów innych niż master
    if (myRank != 0) {
        edges = (int*)malloc(numE * 3 * sizeof(int));
    }

    // Przekazanie krawędzi wszystkim procesom
    MPI_Bcast(edges, numE * 3 * sizeof(int), MPI_BYTE, 0, world);

    // Wyliczenie zakresu krawędzi dla danego procesu
    int edgesPerProcess = numE / numProc;
    int rem = numE % numProc;
    int startEdge = myRank * edgesPerProcess;
    int endEdge = startEdge + edgesPerProcess;
    if (myRank == numProc - 1) {
        endEdge += rem;
    }    

    // Alokacja pamięci na tablice odległości
    int* dist = (int*)malloc(numV * sizeof(int));
    int* localDist = (int*)malloc(numV * sizeof(int));

    // Inicjalicacja tablicy odległości dla procesu master
    if(myRank == 0) {
        for (int i = 0; i < numV; i++) {
            dist[i] = INF;
        }
        dist[source] = 0;
    }

    // Pętla głowna algorytmu
    for (int i = 0; i < numV - 1; i++) {
        // Przekazanie tablicy odległości do wszystkich procesów
        MPI_Bcast(dist, numV, MPI_INT, 0, world);
        // Skopiowanie tablicy odległości do tablicy lokalnej
        for (int i = 0; i < numV; i++) {
            localDist[i] = dist[i];
        }
        // Obliczenie nowych odległości
        for (int j = startEdge; j < endEdge; j++) {
            int u = edges[j * 3];
            int v = edges[j * 3 + 1];
            int w = edges[j * 3 + 2];

            if (localDist[u] + w < localDist[v]) {
                localDist[v] = localDist[u] + w;
            }
        }
        // Zbieranie wyników
        MPI_Reduce(localDist, dist, numV, MPI_INT, MPI_MIN, 0, world);
    }

    // Sprawdzenie czy graf zawiera ujemny cykl
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

    if (myRank == 0) {
        // zakonczenie pomiaru czasu
        timer_end = MPI_Wtime();

        // Pokazanie wyników przez proces master
        if (hasNegativeCycle) {
            printf("Graph contains negative cycle\n");
        } else {
            for (int i = 0; i < numV; i++) {
                printf("Distance from %d to %d: %d\n", source, i, dist[i]);
            }
        }

        printf("Time: %f\n", timer_end - timer_start);
    }

    // Zwolnienie pamięci
    free(edges);
    free(dist);
    free(localDist);

    // Zakończenie MPI
    MPI_Finalize();
    return 0;
}
