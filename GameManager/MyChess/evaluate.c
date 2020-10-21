#include "defs.h"

// include NNUE probe library wrapper header
#include "nnue_eval.h"

// Penalita' per il punteggio di un pedone isolato

int PawnIsolated = -10;



// Bonus per il punterggio del pedone a seconda del RANK (i valori dei rank 0 e 7 non vengono utilizzati)
// LA tabella si riferisce ad un pedone del Bianco (per il nero bisogna simmetrizzare)

int PawnPassed[] = { 0, 5, 10, 20, 35, 60, 100, 200 };



// Bonus per il punteggio di una Torre su una colonna aperta

int RookOpenFile = 10;

// Bonus per il punteggio di una Torre su una colonna semi-aperta

int RookSemiOpenFile = 5;

// Bonus per il punteggio di una Regina su una colonna aperta

int QueenOpenFile = 5;

// Bonus per il punteggio di una Regina su una colonna semi-aperta

int QueenSemiOpenFile = 3;

// Bonus per la coppia di Alfieri

int BishopPair = 30;



//
// Le tabelle che seguono si riferiscono rispettivamente
// al Pedone, al Cavallo, all'Alfiere e alla Torre.
//
// In pratica assegnano un punteggio in centesimi di pedone
// al pezzo, in base alla casella di appartenenza della
// scacchiera REALE.
//
// Si valuta percio' che un pezzo abbia una forza che
// dipende dalla posizione che nella scacchiera.
//
// NOTA IMPORTANTE:
//
// I punteggi si riferiscono a pezzi del Bianco.
// I punteggi per il Nero per il pezzo in una casella
// si ottiene valutando il punteggio del Bianco NELLA
// CASELLA SIMMETRICA rispetto un asse di simmetria orizzontale
// che passa per il centro scacchiera e invertendo tale punteggio.
// La casella simmetrica viene ottenuta mediante la tabella Mirror64


int PawnTable[] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

int KnightTable[] = {
0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0		
};

int BishopTable[] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

int RookTable[] = {
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0		
};

// Tabella per il Re per partita in fase di apertura

int KingO[] = {	
0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70		
};

// Tabella per il Re per partita in fase di finale

int KingE[] = {	
-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	
};

// Questa macro definisce il materiale equivalente al di sotto del quale si
// considera il gioco nella fase del finale di partita
// IL punteggio e' equivalente a quello di una Torre 2 Cavalli 2 pedoni

#define	ENDGAME_MAT		(1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP])



//
// Funzione per valutare se il materiale e' pari
//
// Come si vede nella valutazione si tiene conto solo delle figure
// e comunque se c'e' almeno una regina in gioco il materiale
// non e' mai giudicato pari
//
// QUINDI QUESTA VALUTAZIONE E' FATTA PER UNA POSIZIONE SENZA PEDONI
//
// INPUT:		pos		pointer alla posizione
//				TRUE	il materiale e' pari

int MaterialDraw(S_BOARD *pos) {

	assert(CheckBoard(pos));				// Controlla coerenza della posizione
	
    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {		// Se non ci sono ne' Regine ne' Torri

		if (!pos->pceNum[bB] && !pos->pceNum[wB]) {											// Se non ci sono neanche Alfieri

	      if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3)									// E se ciascuna parte ha meno di 3 Cavalli
			  return TRUE;																	// Allora il materiale e' pari
		}
		else
			if (!pos->pceNum[wN] && !pos->pceNum[bN]) {										// Se invece non ci sono Cavalli

				if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2)								// Se una parte ha al piu' solo un Alfiere di vantaggio
					return TRUE;															// Allora il materiale e' pari
			}
			else																			// Se invece ci sono Alfieri e Cavalli
				if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {		// Se il Bianco ha meno di 3 Cavalli e non ha Alfieri oppure ha solo un Alfiere e nessun Cavallo 

					if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))	// e il Neroo ha meno di 3 Cavalli e non ha Alfieri oppure ha solo un Alfiere e nessun Cavallo
						return TRUE;														// Allora il materiale e' pari
				}
	}
	else
		if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {											// Se ci sono Torri ma comuqnue non Regine

			if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {								// Se ogni parte ha una sola Torre

				if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)		// e la somma dei Cavalli e degli Alfieri di ciscauna parte non supera 2
					return TRUE;															// Allora il materiale e' pari
			}
			else
				if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {								// Se invece solo il Bianco ha la Torre

					if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) // e il Bianco non ha ne' Cavalli ne' Alfieri
																																								// ed il Nero ha una somma di Cavalli e Alfieri
																																								// compresa tra 1 e 2
						return TRUE;																															// Allora il materiale e' pari
				}
				else
					if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {							// Se e' invece solo il Nero ad avere la Torre
            
						if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2)))		// e il Nero non ha ne' Cavalli ne' Alfieri
																																										// ed il Biancoo ha una somma di Cavalli e Alfieri
																																										// compresa tra 1 e 2
							return TRUE;																																// Allora il materiale e' pari
					}
		}

  return FALSE;
}

// Stockfish NNUE piece encoding
int nnue_pieces[13] = {0, 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7};

//
// Funzione di valutazione di una posizione
//
// Il punteggio e' relativo alla parte del Bianco
//
// INPUT:		RawEval			Flag per valutazione grezza
// OUTPUT:						punteggio in centesimi di pedone

int EvalPositionW(int RawEval) {

	int pceNum;					// Indice in loop del pezzo
	int sq;						// Casella in cui il pezzo e' presente
	int score;					// punteggio della posizione
	int scoreW;					// punteggio del materiale Bianco
	int scoreB;					// punteggio del materiale Nero

	scoreW = 0;					// Inizializza il contributo del Bianco
	scoreB = 0;					// Inizializza il contributo del Nero

	if (RawEval)																		// Test valutazione grezza attiva
		return (pos->material[WHITE] - pos->material[BLACK]);

	if ( ( (pos->pceNum[wP] == 0) && (pos->pceNum[bP] == 0) ) && MaterialDraw(pos))		// Se posizione senza pedoni e materiale equivalente segnala punteggio pari
		return 0;

	// Pedoni del Bianco

	for (pceNum = 0; pceNum < pos->pceNum[wP]; pceNum++) {			// Loop su tutti i Pedoni del Bianco
	
		sq = pos->pList[wP][pceNum];								// Ricava casella nella scacchiera estesa del Pedone Bianco interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreW += PawnTable[SQ64(sq)];								// Aggiunge al punteggio del Bianco il contributo del Pedone in base alla sua posizione

		// Penalty per il pedone isolato

		if ( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0 ) {	// Se pedone isolato
		
//			DoLog("wP Iso: %s\n", PrSq(sq));

			scoreW += PawnIsolated;									// Ricordiamo che PawnIsolated e' negativo
		
		}

		// Bonus per il pedone passato

		if ( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0 ) {	// Se pedone passato
		
//			DoLog("wP Passed: %s\n", PrSq(sq));

			scoreW += PawnPassed[RanksBrd[sq]];							// Il bonus e' modulato a seconda della traversa su cui si trova il pedone
		}
	}



	// Pedoni del Nero

	for (pceNum = 0; pceNum < pos->pceNum[bP]; pceNum++) {			// Loop su tutti i Pedoni del Nero
	
		sq = pos->pList[bP][pceNum];								// Ricava casella nella scacchiera estesa del Pedone Nero interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreB += PawnTable[MIRROR64(SQ64(sq))];					// Aggiunge al punteggio del Nero il contributo del Pedone in base alla sua posizione
																	// utilizzando la tabella dei punteggi del Bianco, accedendo con la casella simmetrizzata
		
		// Penalty per il pedone isolato

		if ( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0 ) {	// Se pedone isolato
		
//			DoLog("bP Iso: %s\n", PrSq(sq));

			scoreB += PawnIsolated;									// Ricordiamo che PawnIsolated e' negativo
		
		}

		// Bonus per il pedone passato

		if ( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0 ) {	// Se pedone passato
		
//			DoLog("bP Passed: %s\n", PrSq(sq));

			scoreB += PawnPassed[7 - RanksBrd[sq]];						// Il bonus e' modulato a seconda della traversa su cui si trova il pedone (l'accesso al vettore PawnPassed e' simmetrizzato)
		}
	}


	// Cavalli del Bianco

	for (pceNum = 0; pceNum < pos->pceNum[wN]; pceNum++) {			// Loop su tutti i Cavalli del Bianco
	
		sq = pos->pList[wN][pceNum];								// Ricava casella nella scacchiera estesa del Cavallo Bianco interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreW += KnightTable[SQ64(sq)];							// Aggiunge al punteggio del Bianco il contributo del Cavallo in base alla sua posizione
	}

	// Cavalli del Nero

	for (pceNum = 0; pceNum < pos->pceNum[bN]; pceNum++) {			// Loop su tutti i Cavalli del Nero
	
		sq = pos->pList[bN][pceNum];								// Ricava casella nella scacchiera estesa del Cavallo Nero interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreB += KnightTable[MIRROR64(SQ64(sq))];					// Aggiunge al punteggio del Nero il contributo del Cavallo in base alla sua posizione
																	// utilizzando la tabella dei punteggi del Bianco, accedendo con la casella simmetrizzata
	}


	// Alfieri del Bianco

	for (pceNum = 0; pceNum < pos->pceNum[wB]; pceNum++) {			// Loop su tutti gli Alfieri del Bianco
	
		sq = pos->pList[wB][pceNum];								// Ricava casella nella scacchiera estesa dell'Alfiere Bianco interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreW += BishopTable[SQ64(sq)];							// Aggiunge al punteggio del Bianco il contributo dell'Alfiere in base alla sua posizione
	}

	if (pos->pceNum[wB] >=2)
		scoreW += BishopPair;										// Bonus per la coppia degli Alfieri

	// Alfieri del Nero

	for (pceNum = 0; pceNum < pos->pceNum[bB]; pceNum++) {			// Loop su tutti gli Alfieri del Nero
	
		sq = pos->pList[bB][pceNum];								// Ricava casella nella scacchiera estesa dell'Alfiere Nero interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreB += BishopTable[MIRROR64(SQ64(sq))];					// Aggiunge al punteggio del Nero il contributo dell'Alfiere in base alla sua posizione
																	// utilizzando la tabella dei punteggi del Bianco, accedendo con la casella simmetrizzata
	}
	
	if (pos->pceNum[bB] >=2)
		scoreB += BishopPair;										// Bonus per la coppia degli Alfieri

	// Torri del Bianco

	for (pceNum = 0; pceNum < pos->pceNum[wR]; pceNum++) {			// Loop su tutte le Torri del Bianco
	
		sq = pos->pList[wR][pceNum];								// Ricava casella nella scacchiera estesa della Torre Bianca interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreW += RookTable[SQ64(sq)];								// Aggiunge al punteggio del Bianco il contributo dela Torre in base alla sua posizione

		// Bonus per Torre su colonna aperta

		if ( (pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone sulla stessa colonna della Torre (ovvero se Torre su colonna aperta)
			scoreW += RookOpenFile;
		else

		// Bonus per Torre su colonna semi-aperta

		if ( (pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone Bianco sulla stessa colonna della Torre (ovvero se Torre su colonna semi-aperta)
			scoreW += RookSemiOpenFile;
	}

	// Torri del Nero

	for (pceNum = 0; pceNum < pos->pceNum[bR]; pceNum++) {			// Loop su tutte le Torri del Nero
	
		sq = pos->pList[bR][pceNum];								// Ricava casella nella scacchiera estesa della Torre Nera interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		scoreB += RookTable[MIRROR64(SQ64(sq))];					// Aggiunge al punteggio del Nero il contributo della TOrre in base alla sua posizione
																	// utilizzando la tabella dei punteggi del Bianco, accedendo con la casella simmetrizzata

		// Bonus per Torre su colonna aperta

		if ( (pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone sulla stessa colonna della Torre (ovvero se Torre su colonna aperta)
			scoreB += RookOpenFile;
		else

		// Bonus per Torre su colonna semi-aperta

		if ( (pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone Nero sulla stessa colonna della Torre (ovvero se Torre su colonna semi-aperta)
			scoreB += RookSemiOpenFile;
	}

	// Regine del Bianco

	for (pceNum = 0; pceNum < pos->pceNum[wQ]; pceNum++) {			// Loop su tutte le Regine del Bianco
	
		sq = pos->pList[wQ][pceNum];								// Ricava casella nella scacchiera estesa della Regina Bianca interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		// Bonus per Regina su colonna aperta

		if ( (pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone sulla stessa colonna della Regina (ovvero se Regina su colonna aperta)
			scoreW += QueenOpenFile;
		else

		// Bonus per Regina su colonna semi-aperta

		if ( (pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone Bianco sulla stessa colonna della Regina (ovvero se Regina su colonna semi-aperta)
			scoreW += QueenSemiOpenFile;
	}

	// Regine del Nero

	for (pceNum = 0; pceNum < pos->pceNum[bQ]; pceNum++) {			// Loop su tutte le Regine del Nero
	
		sq = pos->pList[bQ][pceNum];								// Ricava casella nella scacchiera estesa della Regina Nera interessato

		assert(SqOnBoard(sq));										// Controllo validita' della casella

		// Bonus per Regina su colonna aperta

		if ( (pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone sulla stessa colonna della Regina (ovvero se Regina su colonna aperta)
			scoreB += QueenOpenFile;
		else

		// Bonus per Torre su colonna semi-aperta

		if ( (pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]]) == 0)	// Se nessun Pedone Nero sulla stessa colonna della Regina (ovvero se Regina su colonna semi-aperta)
			scoreB += QueenSemiOpenFile;
	}

	// Contributo della posizione del Re in base alla posizione
	// Si distinguono 2 casi : finale di partita nel quale il Re viene valorizzato al centro
	//						   apertura o mediogioco nel quale il massimo punteggio viene dato nelle posizioni di arrocco

	// Re Bianco
	
	sq = pos->pList[wK][0];										// Ricava casella occupata da Re Bianco

	assert(SqOnBoard(sq));										// Controllo validita' della casella

	if (pos->material[BLACK] <= ENDGAME_MAT)					// Se il punteggio del Nero non supera quello del materiale equivalente a una Torre + 2 Cavalli + 2 Pedoni
		scoreW += KingE[SQ64(sq)];								// Allora giudica di essere in un finale e utilizza l'opportuna tabella degli score per il Re in base alla posizione
	else
		scoreW += KingO[SQ64(sq)];								// Altrimenti giudica di essere in apertura o mediogioco e utilizza l'opportuna tabella degli score per il Re in base alla posizione
	
	// Re Nero
	
	sq = pos->pList[bK][0];										// Ricava casella occupata da Re Nero

	assert(SqOnBoard(sq));										// Controllo validita' della casella

	if (pos->material[WHITE] <= ENDGAME_MAT)					// Se il punteggio del Bianco non supera quello del materiale equivalente a una Torre + 2 Cavalli + 2 Pedoni
		scoreB += KingE[MIRROR64(SQ64(sq))];					// Allora giudica di essere in un finale e utilizza l'opportuna tabella degli score per il Re in base alla posizione
	else
		scoreB += KingO[MIRROR64(SQ64(sq))];					// Altrimenti giudica di essere in apertura o mediogioco e utilizza l'opportuna tabella degli score per il Re in base alla posizione

	// Come si vede non si calcola il contributo della Regina, poiche' essa
	// e' il pezzo piu' potente e la sua forza dipende molto poco dalla sua posizione

	score = pos->material[WHITE] + scoreW - ( pos->material[BLACK] + scoreB );		// Punteggio della posizione calcolato DALLA PARTE DEL BIANCO utilizzando il contributo
																					// indipendente dalla posizione dei pezzi  (material) e il contributo calcolato sopra
																					// che dipende dalla posizione

	return score;													// Out punteggio dalla parte del Bianco

}


//
// Funzione di valutazione di una posizione
//
// Il punteggio e' relativo alla parte che deve muovere
//
// INPUT:			RawEval		flag per valutazione grezza
// OUTPUT:						punteggio in centesimi di pedone

int EvalPositionClassic(int RawEval) {

	int score;

	score = EvalPositionW(RawEval);									// Valuta la posizione dalla parte del Bianco

	if (pos->side == BLACK)											// Poiche' bisogna fornire il punteggio RISPETTO LA PARTE CHE HA LA MOSSA, se e' il Nero a muovere esso viene invertito
		score = -score;

	return score;													// Out punteggi
}

// NNUE

int EvalPosition(int RawEval)
{
if (GetOpt(USENNUE)==0)
	return EvalPositionClassic(RawEval);
else
	{
	// NNUE probe arrays
	int pieces[33];
	int squares[33];
    
	// NNUE probe arrays index
	int index = 2;
    
	// loop over the pieces
	for (int piece = 1; piece < 13; piece++)
		{
    		// loop over the corresponsding squares
    		for(int pceNum = 0; pceNum < pos->pceNum[piece]; ++pceNum)
    			{            
        		// case white king
        		if (piece == wK)
        			{
            			// init pieces & squares arrays
            			pieces[0] = nnue_pieces[piece];
            			squares[0] = SQ64(pos->pList[piece][pceNum]);
        			}
            
        		// case black king
        		else if (piece == bK)
        			{
            			// init pieces & squares arrays
            			pieces[1] = nnue_pieces[piece];
            			squares[1] = SQ64(pos->pList[piece][pceNum]);
        			}
            
        		// all the other pieces regardless of order
        		else
        			{
            			// init pieces & squares arrays
            			pieces[index] = nnue_pieces[piece];
            			squares[index] = SQ64(pos->pList[piece][pceNum]);
            
            			// increment the index
            			index++;
        			}
    			}
    
		}    
    
    	// end piece and square arrays with zero terminating characters
    	pieces[index] = 0;
    	squares[index] = 0;
    
    	return evaluate_nnue(pos->side, pieces, squares);
	}
}
