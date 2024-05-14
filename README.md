# Routing w sieciach komputerowych - algorytm Bellmana-Forda.
## Alan Guzek i Michał Sienkiewicz

Implementacja rozproszonego algorytmu Bellmana-Forda przy użyciu MPI w języku C. 
Program liczy na podstawie pliku wejściowego zawierającego informacje o grafie najmniejsze odległości pomiędzy wybranym wierzchołkiem source, a innymi wierzchołkami.

Format pliku wejściowego:
```
<ilość wierzchołków> <ilość krawędzi>
<wierzchołek początkowy> <wierzchołek końcowy> <waga>
<wierzchołek początkowy> <wierzchołek końcowy> <waga>
...
```

Kompilacja programu: 
`mpicc bellmanford.c -o bellmanford`
Uruchomienie programu: 
`mpiexec -n 4 -f <plik z nodami> ./bellmanford <plik z danymi> <wierzchołek źródłowy>`

Można również użyć pliku `run.sh`, który uruchomi program na 4 nodach.

Schemat działania programu:

- Inicjalizacja i finalizacja MPI klasyczna. Używany jest komunikator `MPI_COMM_WORLD`.
- Funkcja `readFile()` jest wykorzystana do sczytywania danych z pliku.
- W programie został wykorzystany `MPI_Wtime()` do obserwacji czasu wykonania.
- Do każdego węzła zostały rozesłane informacje, z wykorzystaniem funkcji `MPI_Bcast()` na temat liczby wierzchołków i krawędzi grafu oraz tablica krawędzi. 
- Krawędź są to kolejne trzy liczby: `u` (indeks wierzchołka początkowego), `v` (indeks wierzchołka końcowego), `w` (waga na krawędzi).
Zakres krawędzi dla każdego wierzchołka wyliczany jest jako ilość `wierzchołków` / `ilość procesów`.
- Zostały zaalokowane tablice dystansów: globalna oraz lokalna wykorzystywana do zbierania wyników z poszczególnych węzłów.
Relaksacja krawędzi grafu z podzieleniem pracy na poszczególne procesy.
Liczba iteracji jest równa ilości wierzchołków pomniejszonej o 1.
    - Każdy wątek sprawdzał, czy istnieje mniejsza odległość od źródła do końca
    - Po każdej iteracji nastąpiło zebranie wyników ze wszystkich wątków i zapisanie najmniejszych wartości do tablicy.
- Sprawdzenie czy w grafie występuje cykl negatywny.
- Przedstawienie wyników wraz z czasem wykonania programu.
- Zwolnienie zaalokowanej pamięci oraz zakończenie pracy MPI.
