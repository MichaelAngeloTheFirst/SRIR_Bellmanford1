#include "mpi.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <stdexcept>

#define INF 1000000000

using Edge = std::array<int, 3>;
using Edges = std::vector<Edge>;
using Results = std::tuple<std::vector<int>, std::vector<int>>;

int u(const Edge& edge) { return edge[0]; }
int v(const Edge& edge) { return edge[1]; }
int w(const Edge& edge) { return edge[2]; }

int getEdges(const std::string& filename, Edges& edges);
Results seq_bellman_ford(const Edges& edges, int n, int source);
Results par_bellman_ford(const Edges& edges, int n, int source, MPI_Comm comm, int myRank, int numProc);
void print_result(const Results& results, double start_time, double end_time);


int main(int argc, char** argv){

    if(argc <=1 ){
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return -1;
    }

    // BEGIN: Setup MPI
    MPI_Comm world;
    int numProc;
    int myRank;
    
    MPI_Init( &argc, &argv );
    world  = MPI_COMM_WORLD;
    MPI_Comm_size( world, &numProc);
    MPI_Comm_rank( world, &myRank);

    double timer_start, timer_end;
    Results results;

    Edges edges;
    int n = getEdges(argv[1], edges);
    // END

    std::cout << "Sequential Bellman Ford: " << std::endl;

    // BEGIN: Sequential Bellman-Ford Algorithm
    MPI_Barrier(world);
    timer_start = MPI_Wtime();

    results = seq_bellman_ford(edges, n, 0);
    
    MPI_Barrier(world);
    timer_end = MPI_Wtime();

    if (myRank == 0) {
       print_result(results, timer_start, timer_end);
    }
    // END

    std::cout << "Parallel Bellman Ford: " << std::endl;

    // BEGIN: Prallel Bellman-Ford Algorithm
    MPI_Barrier(world);
    timer_start = MPI_Wtime();

    results = par_bellman_ford(edges, n, 0, world, myRank, numProc);
    
    MPI_Barrier(world);
    timer_end = MPI_Wtime();

    if (myRank == 0) {
       print_result(results, timer_start, timer_end);
    }
    // END

    MPI_Finalize();
    return 0;

}

int getEdges(const std::string& filename, Edges& edges) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file " + filename);
    }
    
    int NumV, NumE;
    file >> NumV >> NumE;
    edges.resize(NumE);
    for (auto& edge : edges){
        file >> edge[0] >> edge[1] >> edge[2];
    }
    file.close();

    return NumV;
}


Results seq_bellman_ford(const Edges& edges, int n, int source){
    auto dist = std::vector<int>(n, INF); // Tablica odległości od źródła
    auto p = std::vector<int>(n, -1);  // Tablica poprzedników

    // Odległość od źródła do samego siebie wynosi 0
    dist[source] = 0;

    // Pętla relaksacji
    for (int i = 0; i < n-1; i++){
        // Dla każdej krawędzi sprawdzamy czy można skrócić odległość
        for (auto edge : edges){
            if (dist[u(edge)] + w(edge) < dist[v(edge)]){
                dist[v(edge)] = dist[u(edge)] + w(edge);
                p[v(edge)] = u(edge);
            }
        }
    }

    // Sprawdzamy czy nie ma cyklu ujemnego
    for (auto edge : edges){
        if (dist[u(edge)] + w(edge) < dist[v(edge)]){
            throw std::runtime_error("Negative cycle detected");
        }
    }

    return {dist, p};
}

Results par_bellman_ford(const Edges& edges, int n, int source, MPI_Comm comm, int myRank, int numProc){
    auto dist = std::vector<int>(n, INF); // Tablica odległości od źródła
    auto p = std::vector<int>(n, -1);  // Tablica poprzedników

    // Odległość od źródła do samego siebie wynosi 0
    dist[source] = 0;

    // Lokalne wartości
    int verticesPerProcess = n / numProc;
    int remainder = n % numProc;
    int startVertex = myRank * verticesPerProcess;
    int endVertex = startVertex + verticesPerProcess;

    // Wyrównanie ze względu na resztę z dzielenia
    if (myRank == numProc - 1) {
        endVertex += remainder;
    }

    // Perform parallel Bellman-Ford algorithm
    for (int k = 0; k < n - 1; k++) {
        // Broadcast the current distance array
        MPI_Bcast(&dist[0], n, MPI_INT, 0, comm);

        // Relax edges in parallel
        for (auto edge : edges) {
            if (u(edge) >= startVertex && u(edge) < endVertex) {
                if (dist[u(edge)] + w(edge) < dist[v(edge)]) {
                    dist[v(edge)] = dist[u(edge)] + w(edge);
                    p[v(edge)] = u(edge);
                }
            }
        }

        // Gather the updated distance array
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, &dist[0], verticesPerProcess, MPI_INT, comm);
    }

    return {dist, p};
}

void print_result(const Results& results, double start_time, double end_time){
    auto dist = std::get<0>(results);
    auto p = std::get<1>(results);

    for (int i = 0; i < dist.size(); i++){
        printf("Distance from 0 to %d: %d\n", i, dist[i]);
    }
    printf("Time: %f\n", end_time - start_time);
}

