mpicc bellmanford.c -o bellmanford.out
chmod +x bellmanford.out
mpiexec -n 4 -f nodes ./bellmanford.out input.txt