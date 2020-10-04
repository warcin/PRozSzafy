#ifndef CONSTSH
#define CONSTSH


#define ROOMS 4					// liczba pomieszczeń
#define ELEVATORS 2				// liczba wind
#define MAX_ROOMS 4				// maksymalna ilość używanych pomieszczeń przez proces

#define STATE_CHANGE_PROB 30    // prawdopodobieństwo zmiany stanu IDLE (chęci wejścia do pokoju)
#define SEC_IN_STATE 1          // liczba sekund spędzonych w jednym stanie
#define SEC_IN_ELEVATOR 3		// liczba sekund spędzonych w windzie (musi być większe od SEC IN STATE)
#define MIN_SEC_IN_ROOM 3		// minimalna liczba dodatkowych sekund spędzonych w pomieszczeniu (musi być większe od SEC IN STATE)
#define MAX_SEC_IN_ROOM 5		// maksymalna liczba dodatkowych sekund spędzonych w pomieszczeniu (musi być większe od MIN SEC IN ROOM)
#define LAMPORT_DEFAULT 0       // początkowa wartość zegara Lamporta

#endif

#ifndef DEBUG
#define DEBUG true

#endif