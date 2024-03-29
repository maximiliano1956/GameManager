#include "Defines.h"
#include "extern.h"

int Min1(int depth) {

	MList list;
	int MoveNum;
	int alfa;
	int score;
	CompMove BestMove;

	char moveBuffer[MAXLMOVE];
	DLL_Result esito;
	
	CheckUp();                                              // controlla se forzare fine ricerca
	
	info->nodes++;											// Aggiorna il numero dei nodi visitati nella ricerca
	*info->plNodes = (long)info->nodes;						// Per la GUI

	if ( (pIsDraw && (*pIsDraw)()) && (*pGetPly)() )		// Test se posizione pari per ripetizione  o per regola delle 50 mosse)  0 = punteggio di patta  
		return DRAW_SCORE;									// A livello di root (primo loop iterative deepening) si va' comunque oltre in modo da caricare almeno la prima mossa
															//  della variante principale

	if (depth <= 0) {
		score = (*pEval)(EngineOptions->RawEval);

		return -score;										// Score sempre dalla parte del Bianco
	}

	(*pGenMoveAllowed)(&list,FALSE);						// Genera la lista delle mosse ammissibili nella posizione

	GenStrMoves(&list);										// Genera stringhe mosse nel formato ZoG

	alfa = WIN_SCORE;

	for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++) {
		
		strcpy(moveBuffer,list.m[MoveNum].str_moves);		// Copia òa stringa della mossa nel formato Zog in moveBuffer

		esito = DoMove(moveBuffer);							// Esegue la mossa

		assert(esito == DLL_OK);							// Controllo se mossa corretta

		score = Max1(depth -1);

		if (score < alfa) {
			alfa = score;
			BestMove = list.m[MoveNum];
		}

		(*pTakeBack)();											// Si torna indietro alla posizione in input
			
		if (info->stopped == TRUE && PvArray.nel)				// Test se interrompere la ricerca per tempo scaduto
			return 0;											// Non ha importanza
	}

	// Controllo scaccomatto/stallo

	if ((*pCheckLoss)())
		alfa = WIN_SCORE - (*pGetPly)();				// si ha lo scacco matto e si esce con un punteggio negativissimo contenente in qualmodo informazione sulla distanza di semimosse dal matto
	else
		if ((*pCheckDraw)())
			alfa = DRAW_SCORE;							// Se il Re non e' in scacco e' Stallo: si esce col punteggio di parita'
		else
			if (alfa != WIN_SCORE)
				StoreHashEntry((*pGetHashKey)(),BestMove,score,HFEXACT,depth);

	return alfa;
}



int Max1(int depth) {

	MList list;
	int MoveNum;
	int alfa;
	int score;
	CompMove BestMove;

	char moveBuffer[MAXLMOVE];
	DLL_Result esito;
	
	CheckUp();                                              // controlla se forzare fine ricerca
	
	info->nodes++;											// Aggiorna il numero dei nodi visitati nella ricerca
	*info->plNodes = (long)info->nodes;						// Per la GUI

	if ( (pIsDraw && (*pIsDraw)()) && (*pGetPly)() )		// Test se posizione pari per ripetizione  o per regola delle 50 mosse)  0 = punteggio di patta  
		return DRAW_SCORE;									// A livello di root (primo loop iterative deepening) si va' comunque oltre in modo da caricare almeno la prima mossa
															//  della variante principale

	if (depth <= 0) {
		score = (*pEval)(EngineOptions->RawEval);

		return score;
	}

	(*pGenMoveAllowed)(&list,FALSE);						// Genera la lista delle mosse ammissibili nella posizione

	GenStrMoves(&list);										// Genera stringhe mosse nel formato ZoG

	alfa = LOSS_SCORE;

	for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++) {
		
		strcpy(moveBuffer,list.m[MoveNum].str_moves);		// Copia òa stringa della mossa nel formato Zog in moveBuffer

		esito = DoMove(moveBuffer);							// Esegue la mossa

		assert(esito == DLL_OK);							// Controllo se mossa corretta

		score = Min1(depth -1);

		if (score > alfa) {
			alfa = score;
			BestMove = list.m[MoveNum];
		}

		(*pTakeBack)();											// Si torna indietro alla posizione in input
			
		if (info->stopped == TRUE && PvArray.nel)				// Test se interrompere la ricerca per tempo scaduto
			return 0;											// Non ha importanza
	}

	if ((*pCheckLoss)())
		alfa = LOSS_SCORE + (*pGetPly)();				// si ha lo scacco matto e si esce con un punteggio negativissimo contenente in qualmodo informazione sulla distanza di semimosse dal matto
	else
		if ((*pCheckDraw)())
			alfa = DRAW_SCORE;							// Se il Re non e' in scacco e' Stallo: si esce col punteggio di parita'
		else
			if (alfa != LOSS_SCORE)
				StoreHashEntry((*pGetHashKey)(),BestMove,score,HFEXACT,depth);

	return alfa;
}

