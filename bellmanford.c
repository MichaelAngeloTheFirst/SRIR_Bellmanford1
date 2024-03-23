#include "mpi.h"
#include <stdio.h>


#define INF 1000000000
#define N 5  


int read_file(char * filename, int * result){
    // int * mat = (int*) malloc(N*N*sizeof(int));
    static int mat[N][N] = {
        {INF,INF,4, INF,5},
        {INF,INF,-4,INF,INF},
        {-3,INF,INF,INF,INF},
        {4,INF,7,INF,3},
        {INF,2,3,INF,INF}
    };

    result = mat;
    return 0;
}

int  main(int argc, char** argv){

    if(argc <=1 ){
        printf("File not found...");
        return -1;
    }

    // BEGIN: mpi initialization
    MPI_Comm world;
    int numProc;
    iny myRank;
    
    MPI_Init( &argc, &argv );
    world  = MPI_COMM_WORLD;
    MPI_Comm_size( world, &numProc);
    MPI_Comm_rank( world, &myRank);
    // END

    // BEGIN: read data 
    int[N][N] matrix;

    if(myRank == 0){
        if(!read_file("TEST", matrix)){
            printf("File read.");
        }
    }

    // END 

    // BEGIN: BellmanFord algorithm
    //time counter
    double t1, t2;
    MPI_Barrier(world);
    t1 = MPI_Wtime();

    //bellman-ford algorithm
    // TODO: IMPLEMENT
    
    MPI_Barrier(world);

    //end timer
    t2 = MPI_Wtime();
    // END

    // BEGIN: show results
    if (my_rank == 0) {
        printf("Time of execution: %lf.4");
    }
    // END 

    MPI_Finalize();
    return 0;

}