#include "defs.h"

//
// Funzione che effettua alcuni controlli sul contenuto della lista delle mosse ammissibili in una posizione
//
// INPUT:	list		pointer alla lista
//			pos			pointer alla posizione
// OUTPUT:	FALSE		Se lista invalida

int MoveListOk(S_MOVELIST *list,S_BOARD *pos) {

	int MoveNum;		// Indice nella lista

	if (list->count < 0 || list->count >= MAXMOVES)			// Controllo validita' del numero di mosse presenti in lista
		return FALSE;

	for (MoveNum = 0; MoveNum < list->count; MoveNum++) {
	
		if (!SqOnBoard(FROMSQ(list->moves[MoveNum].move)) || !SqOnBoard(TOSQ(list->moves[MoveNum].move)))	// Test validita' caselle di partenza e arrivo nella mossa corrente presente in lista
			return FALSE;

		if (!PieceValid(pos->pieces[FROMSQ(list->moves[MoveNum].move)])) {									// Controllo che il codice del pezzo da muovere dichiarato nella mossa sia corretto
		
			LogBoard();
			return FALSE;
		}
	
	}

	return TRUE;					// La lista e' valida
}


//
//	Funzione che controlla la validita' del numero di una generica casella nella scacchiera estesa
//
// INPUT:	sq			Nr. casella nella scacchiera estesa
// OUTPUT:	FALSE		Nr. casella non valido

int SqIs120(int sq) {

	return ( sq >= 0 && sq < BRD_SQ_NUM ); 
}


//
// Funzione che verifica se la casella della scacchiera estesa e' interna a quella reale
//
// INPUT:	sq	casella in scacchiera estesa
// OUTPUT:	0	casella non appartenente a quella reale
//			1	casella appartenente a quella reale

int SqOnBoard(int sq) {

	return (FilesBrd[sq] != OFFBOARD) ? 1 : 0;		// Si poteva usare anche RanksBrd[sq]
}


//
// Funzione che verifica se il codice della parte (bianco o nero) e' corretta
//
// INPUT:	side	parte da verificare
// OUTPUT:	0		codice parte scorretto
//			1		codice parte corretto

int SideValid(int side) {

	return (side == WHITE || side == BLACK) ? 1 : 0;
}


//
// Funzione che verifica il numero di traversa (o colonna) e' corretto
//
// INPUT:	fr	numero di traversa (o colonna)
// OUTPUT:	0	traversa (o colonna) scorretta
//			1	traversa (o colonna) corretta

int FileRankValid(int fr) {

	return (fr >= 0 && fr <= 7) ? 1 : 0;
}


//
// Funzione che verifica se il codice pezzo e' corretto; si considera valido anche il codice EMPTY (= 0)
//
// INPUT:	pce	codice pezzo
// OUTPUT:	0	codice pezzo non corretto
//			1	codice pezzo corretto

int PieceValidEmpty(int pce) {

	return (pce >= EMPTY && pce <= bK) ? 1 : 0;
}


//
// Funzione che verifica se il codice pezzo e' corretto; il codice EMPTY non e' considerato un codice valido
//
// INPUT:	pce	codice pezzo
// OUTPUT:	0	codice pezzo non corretto
//			1	codice pezzo corretto

int PieceValid(int pce) {

	return (pce >= wP && pce <= bK) ? 1 : 0;
}


