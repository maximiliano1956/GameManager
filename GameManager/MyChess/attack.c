#include "defs.h"

//
// Queste tabelle stabiliscono gli incrementi (algebrici) da dare all'indice di una casella nella scacchiera estesa
// per ottenere una casella contigua raggiungibile da un pezzo che si trovasse in essa.
// Escludendo il pedone (banale) e la Regina (che e' combinazione di Torre e Alfiere) si hanno 4 tabelle:
// una per il Cavallo, una per la Torre, una per l'Alfiere e una per il Re
//
// Ricordiamo che per incrementare/decrementare una traversa bisogna sommare/sottrarre 10
// e che per incrementare/decrementare una colonna bisogna sommare/sottrarre 1
//
// Percio' per il Cavallo:
//
//		**
//		 *
//		 *  2 traverse in piu' e una colonna in meno : + 2*10 - 1 = 19
//
//		 **
//		 *
//		 *  2 traverse in piu' e una colonna in piu' : + 2*10 + 1 = 21
//
//         *
//		 ***  1 traversa in piu' e 2 colonne in piu' : + 10 + 2 = 12
//
//		 ***  1 traversa in meno e 2 colonne in piu' : - 10 + 2 = -8
//         *
//
//		   *
//         *
//		  **  2 traverse in meno e una colonna in meno : - 2*10 - 1 = -21
//
//		   *
//         *
//		   **  2 traverse in meno e una colonna in piu' : - 2*10 + 1 = -19
//
//       *
//		 ***  1 traversa in piu' e 2 colonne in meno : + 10 - 2 = 8
//
//		 ***  1 traversa in meno e 2 colonne in meno : - 10 - 2 = -12
//       *
//
//
//		Per il Re:
//
//		Destra:			una colonna in piu'								= 1
//		Basso-Destra:	una colonna in piu' e una traversa in meno		= + 1 - 10 = -9
//		Basso:			una traversa in meno							= -10
//		Basso-Sinistra	una colonna in meno e una traversa in meno		= - 1 - 10 = -11
//		Sinistra		una colonna in meno								= -1
//		Alto-Sinistra	una colonna in meno e una traversa in piu'		= - 1 + 10 = 9
//		Alto			una traversa in piu'							= 10
//		Alto-Destra		una colonna in piu' e una traversa in piu'		= + 1 + 10 = 11
//
//
//		Per l'Alfiere:
//
//		Basso-Destra:	una colonna in piu' e una traversa in meno		= + 1 - 10 = -9
//		Basso-Sinistra	una colonna in meno e una traversa in meno		= - 1 - 10 = -11
//		Alto-Sinistra	una colonna in meno e una traversa in piu'		= - 1 + 10 = 9
//		Alto-Destra		una colonna in piu' e una traversa in piu'		= + 1 + 10 = 11
//
//
//		Per la Torre:
//
//		Destra:			una colonna in piu'								= 1
//		Basso:			una traversa in meno							= -10
//		Sinistra		una colonna in meno								= -1
//		Alto			una traversa in piu'							= 10
//
//
// Percio':

int KnDir[] = { -21, -19, -12, -8, 8, 12, 19, 21};		// Cavallo
int RkDir[] = { -10, -1, 1, 10 };						// Torre
int BiDir[] = { -11, -9, 9, 11 };						// Alfiere
int KiDir[] = { -11, -10, -9, -1, 1, 9, 10, 11};		// Re


//
// Funzione che ricava se una casella e' attaccata
//
// INPUT:	sq			casella da esaminare (scacchiera estesa)
//			side		parte che attacca
// OUTPUT:	FALSE/TRUE	TRUE se la casella e' attaccata

int SqAttacked(int sq,int side) {

	int dir;				// Indice loop sulle direzioni dei vari tipi di pezzi
	int pce;				// Codice pezzo
	int t_sq;				// Appoggio per input parametro sq della funzione 

	// Controlli su correttezza parametri in input

	assert(SqOnBoard(sq));
	assert(SideValid(side));
	assert(CheckBoard(pos));

	// Controllo se la casella e' attaccata da un pedone avversario

	if (side == WHITE)		// Per essere attaccati da un pedone bianco, esso si deve trovare nella traversa - 1 e colonna -1/+1 rispetto sq
		if (pos->pieces[sq - 11] == wP || pos->pieces[sq - 9] == wP)
			return TRUE;	// Casella attaccata
	if (side == BLACK)		// Per essere attaccati da un pedone nero, esso si deve trovare nella traversa + 1 e colonna -1/+1 rispetto sq
		if (pos->pieces[sq + 9] == bP || pos->pieces[sq + 11] == bP)
			return TRUE;	// Casella attaccata

	// Controllo se la casella e' attaccata da un Cavallo avversario

	for (dir = 0; dir < 8; dir++) {					// Loop sulle 8 direzioni del Cavallo
	
		pce = pos->pieces[sq + KnDir[dir]];			// Ricava il codice del pezzo eventualmente presente nella casella interessata

		if (pce !=OFFBOARD &&						// Se casella valida
			IsKn(pce) && PieceCol[pce] == side)		// ed e' presente un Cavallo della parte che attacca
			return TRUE;							// Casella attaccata
	
	}

	// Controllo se la casella e' attaccata da un pezzo con movimenti ortogonali a distanza (Torre,Regina)

	for (dir = 0; dir < 4; dir++) {						// Loop nelle 4 direzioni
	
		t_sq = sq;										// Salva la casella di partenza

		do {											// Itera nella direzione interessata
			t_sq += RkDir[dir];							// Ricava la prossima casella nella direzione del "raggio"
			assert(SqIs120(t_sq));						// Controllo validita' casella ottenuta
			pce = pos->pieces[t_sq];					// Ricava codice pezzo presente nella casella interessata
			if (pce != OFFBOARD	&&						// Se casella valida
				IsRQ(pce) && PieceCol[pce] == side)		// ed e' presente una Torre o una Regina del colore della parte che attacca
				return TRUE;							// Casella attaccata
		} while (pce == EMPTY);							// Esamina la prossima casella nel raggio se la casella attuale e' vuota
	}

	// Controllo se la casella e' attaccata da un pezzo con movimenti diagonali a distanza (Alfiere,Regina)

	for (dir = 0; dir < 4; dir++) {						// Loop nelle 4 direzioni
	
		t_sq = sq;										// Salva la casella di partenza

		do {											// Itera nella direzione interessata
			t_sq += BiDir[dir];							// Ricava la prossima casella nella direzione del "raggio"
			assert(SqIs120(t_sq));						// Controllo validita' casella ottenuta
			pce = pos->pieces[t_sq];					// Ricava codice pezzo presente nella casella interessata
			if (pce != OFFBOARD &&						// Se casella valida
				IsBQ(pce) && PieceCol[pce] == side)		// ed e' presente un Alfiere o una Regina del colore della parte che attacca
				return TRUE;							// Casella attaccata
		} while (pce == EMPTY);							// Esamina la prossima casella nel raggio se la casella attuale e' vuota
	}

	// Controllo se la casella e' attaccata da un Re avversario

	for (dir = 0; dir < 8; dir++) {					// Loop sulle 8 direzioni del Re
	
		pce = pos->pieces[sq + KiDir[dir]];			// Ricava il codice del pezzo eventualmente presente nella casella interessata

		if (pce != OFFBOARD &&						// Se casella valida
			IsKi(pce) && PieceCol[pce] == side)		// ed  e' presente un Re della parte che attacca
			return TRUE;							// Casella attaccata
	}

	return FALSE;		// Casella non attaccata
}