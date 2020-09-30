#ifndef CONSTSH
#define CONSTSH

#define TUNNELS 2               // liczba tuneli
#define X_CREW_SIZE 5           // wielkość ekipy
#define P_TUNNEL_SIZE 12        // rozmiar tunelu

#define STATE_CHANGE_PROB 30    // prawdopodobieństwo zmiany stanu
#define SEC_IN_STATE 1          // liczba sekund spędzonych w jednym stanie
#define MAX_SEC_IN_TUNNEL 5     // maksymalna liczba sekund spędzonych w tunelu
#define LAMPORT_DEFAULT 0       // początkowa wartość zegara Lamporta

#endif

#ifndef DEBUG
#define DEBUG false

#endif