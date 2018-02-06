#include "defs.h"

//
// L'hashkey viene calcolata come exclusive OR di un insieme di valori casuali a 64 bit legati alla posizione corrente.
//
// L'exclusive OR di 2 interi a n bit si ottiene facendo l'exclusive OR bit a bit.
//
// L'exclusive OR di 2 bit e' il negato della funzione identity:
// la funzione identity vale 1 quando i 2 argomenti sono uguali
// l'ex-OR vale 1 quando i 2 argomenti sono diversi
// l'ex-OR e' equivalente alla somma di 2 bit nella quale il riporto viene scartato:
// 0 ^ 0 = 0
// 0 ^ 1 = 1
// 1 ^ 0 = 1
// 1 ^ 1 = 0
//
// (*)
// L'operazione gode della proprieta' associativa e commutativa:
// Inoltre e' :  a ^ 0 = 0 ^ a = a
//				 a ^ a = 0				per qualunque a (a = 0   o   a = 1)
// (*)

// Supponiamo di avere una posizione di cui abbiamo calcolato l'hashkey HK.
// Se ora viene effettuata una mossa che cattura un pezzo (ad es.)
// non sara' necessario ricalcolare dall'inizio di nuovo l'hashkey:
// Sara' sufficiente annullare il contributo della casella di partenza (ora vuota)
// annullare il contributo del pezzo mangiato e aggiungere il contributo del pezzo mosso nella nuova casella
// In piu' magari sara' necessario annullare il contributo dei permessi di arrocco nella vecchia posizione
// e aggiungere il contributo dei permessi dell'arrocco nella nuova posizione.
// Se a e' il contributo del pezzo mosso nella casella di partenza
// Se b e' il contributo del pezzo mangiato
// Se c e' il contributo del pezzo mosso nella nuova casella
// Se d e' il contributo dei permessi dell'arrocco prima della mossa
// Se e e' il contributo dei permessi dell'arrocco dopo la mossa
// Allora sara':
// Nuova hashkey = HK ^ a ^ b ^ c ^ d ^ e
//
// Si sfruttano le proprieta' (*) ottimizzando quindi il tempo di calcolo della hashkey
//

//
// Funzione che calcola l'hashkey di una posizione
//
// INPUT:	pos		pointer a una struttura S_BOARD
// OUTPUT:	hash key della poiszione a 64 bit
//

U64 GeneratePosKey(const S_BOARD *pos) {

	int sq;			// indice casella in scacchiera estesa
	U64 finalkey;	// la hashkey calcolata
	int piece;		// Codice pezzo in casella corrente

	finalkey = 0;	// Init hashkey

	for (sq = 0; sq < BRD_SQ_NUM; sq++) {			// Loop per le casella della scacchiera estesa
		piece = pos->pieces[sq];					// Ricava codice pezzo in casella
		if (piece != OFFBOARD && piece != EMPTY) {	// Deve essere una casella presente nella scacchiera reale e non vuota
			assert(piece >= wP && piece <= bK);		// Controllo che il codice pezzo sia valido
			finalkey ^= PieceKeys[piece][sq];		// Contributo all'hashkey del tipo di pezzo nella casella
		}
	}

	if (pos->side == WHITE) finalkey ^= SideKey;	// Se muove il bianco considera il contributo all'hash key

	if (pos->enPas != NO_SQ) {									// Se casella en-passant attiva
		assert(pos->enPas >= 0 && pos->enPas < BRD_SQ_NUM);		// Controllo se codice casella valida
		finalkey ^= PieceKeys[EMPTY][pos->enPas];				// Contributo all'hashkey della casella vuota di en-passant
	}

	assert(pos->castlePerm >= 0 && pos->castlePerm <= 15);		// Controllo che il codice sui permessi dell'arrocco sia valido

	finalkey ^= CastleKeys[pos->castlePerm];					// Contributo all'hashkey del codice dei permessi dell'arrocco
		
	return finalkey;											// Fornisce in uscita l'hashkey calcolata
}