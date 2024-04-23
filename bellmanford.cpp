#include "mpi.h"
#include <cstdio>
#include <string>
#include <iostream>


#define INF 1000000000
#define N 5  

struct Matrix{
    int* mat;
    int size;
    Matrix(int size);
    Matrix(const std::string& filename);
    ~Matrix();
    int get(int i, int j) const;
    void set(int i, int j, int value);
};

int bellman_ford(const Matrix& m, MPI_Comm comm, int myRank, int numProc);
void print_result(int dist, double start_time, double end_time);


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

    Matrix matrix(argv[1]);
    // END


    // BEGIN: Bellman-Ford Algorithm
    MPI_Barrier(world);
    timer_start = MPI_Wtime();

    int dist = bellman_ford(matrix, world, myRank, numProc);
    
    MPI_Barrier(world);
    timer_end = MPI_Wtime();
    // END

    if (myRank == 0) {
       print_result(dist, timer_start, timer_end);
    }

    MPI_Finalize();
    return 0;

}


int bellman_ford(const Matrix& m, MPI_Comm comm, int myRank, int numProc){
    int n = m.size;
    int* dist = (int*) malloc(n*sizeof(int));
    for (int i = 0; i < n; i++){
        dist[i] = INF;
    }
    dist[0] = 0;

    for (int i = 0; i < n-1; i++){
        for (int u = 0; u < n; u++){
            for (int v = 0; v < n; v++){
                if (m.get(u, v) != INF){
                    if (dist[u] + m.get(u, v) < dist[v]){
                        dist[v] = dist[u] + m.get(u, v);
                    }
                }
            }
        }
    }

    for (int u = 0; u < n; u++){
        for (int v = 0; v < n; v++){
            if (m.get(u, v) != INF){
                if (dist[u] + m.get(u, v) < dist[v]){
                    return 0;
                }
            }
        }
    }

    return 1;
}

void print_result(int dist, double start_time, double end_time){
    if (dist < 0) {
        std::cout << "Negative cycle detected" << std::endl;
    } else {
        std::cout << "Distance from source to destination: " << dist << std::endl;
    }
    printf("Time: %f\n", end_time - start_time);
}

Matrix::Matrix(int size){
    this->size = size;
    this->mat = (int*) malloc(size*size*sizeof(int));
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            this->set(i, j, INF);
        }
    }
}

Matrix::Matrix(const std::string& filename) {
    this->size = N;
    this->mat = (int*) malloc(N*N*sizeof(int));
    int mat[N][N] = {
        {INF,INF,4, INF,5},
        {INF,INF,-4,INF,INF},
        {-3,INF,INF,INF,INF},
        {4,INF,7,INF,3},
        {INF,2,3,INF,INF}
    };

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            this->set(i, j, mat[i][j]);
        }
    }
}

Matrix::~Matrix(){
    free(mat);
}

void Matrix::set(int i, int j, int value){
    mat[i*size+j] = value;
}

int Matrix::get(int i, int j) const{
    return mat[i*size+j];
}
