mpicc bellmanford.c -o bellmanford.out
mpiexec -n 4 -f nodes ./bellmanford.out input.txt