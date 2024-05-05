mpicc bellmanford.c -o cbel.out
mpiexec -n 3 ./cbel.out input.txt