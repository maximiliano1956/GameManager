#include "Defines.h"

int NegaMax(int depth) {

	MList list;
	int MoveNum;
	int alfa;
	int score;
	CompMove BestMove;

	char moveBuffer[MAXLMOVE];
	DLL_Result esito;
	
	if (( info->nodes & 0x7FF ) == 0)						// Ogni 2048 nodi controlla di non avere superato il tempo limite
		CheckUp();
	
	info->nodes++;											// Aggiorna il numero dei nodi visitati nella ricerca
	*info->plNodes = (long)info->nodes;						// Per la GUI

	if ( (pIsDraw && (*pIsDraw)()) && (*pGetPly)() )		// Test se posizione pari per ripetizione  o per regola delle 50 mosse)  0 = punteggio di patta  
		return DRAW_SCORE;									// A livello di root (primo loop iterative deepening) si va' comunque oltre in modo da caricare almeno la prima mossa
															//  della variante principale

	if (depth <= 0)
		return (*pEval)(EngineOptions->RawEval);

	(*pGenMoveAllowed)(&list,FALSE);						// Genera la lista delle mosse ammissibili nella posizione

	GenStrMoves(&list);										// Genera stringhe mosse nel formato ZoG

	alfa = LOSS_SCORE;

	for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++) {

		strcpy(moveBuffer,list.m[MoveNum].str_moves);		// Copia òa stringa della mossa nel formato Zog in moveBuffer

		esito = DoMove(moveBuffer);							// Esegue la mossa

		assert(esito == DLL_OK);							// Controllo se mossa corretta

		score = -NegaMax(depth -1);

		if (score > alfa) {
			alfa = score;
			BestMove = list.m[MoveNum];
		}

		(*pTakeBack)();										// Si torna indietro alla posizione in input
						
		if (info->stopped == TRUE && PvArray.nel)			// Test se interrompere la ricerca per tempo scaduto
			return 0;										// Non ha importanza
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
