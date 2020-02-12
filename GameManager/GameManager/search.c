#include "Defines.h"
#include "extern.h"

//
// Questa funzione controlla se sospendere la ricerca
// (per limiti di tempo o per iniziativa della GUI)
//

void CheckUp(void) {
	if (!info->stopped)
	{
		if (!UciMode && (*info->status != kKEEPSEARCHING))
			 info->stopped = TRUE;

	        if (info->timeset == TRUE && info->stoptime != INFINITE && GetTimeMs() > info->stoptime)	// Se e' settato un tempo per la ricerca ed e' scaduto
                	info->stopped = TRUE;									// segnala di uscire dall'iterative deepening della ricerca

		if (UciMode && (( info->nodes % 100 ) == 0) && InputWaiting())
			info->stopped = TRUE;

		if (info->stopped == TRUE)
			DoLog("Stopped\n");
	}
}

//
// Questa funzione scambia 2 elementi della lista delle mosse
// in modo tale che la prossima mossa sia quella che ha il punteggio maggiore
//
// INPUT:	moveNum		indice nella lista della prossima mossa
//			list		pointer alla lista

void PickNextMove(int moveNum,MList *list) {

	int index;				// indice nella lista
	int bestScore;			// Migliore punteggio trovato
	int bestNum;			// Indice della mossa con il migliore punteggio
	CompMove temp;			// Appoggio per lo scambio delle mosse nella lista

	assert(moveNum >= 0 && moveNum < list->nmoves);		// Controllo validita' indice mossa in input

	bestScore = 0;			// Valore di default per la ricerca del massimo punteggio
	bestNum = moveNum;		// Cosi' se nessun score supera 0 non avviene nessuno scambio nella lista 

	for (index = moveNum; index < list->nmoves; index++)	// Cerca nella lista, a partire dalla posizione della mossa che verra' giocata nella ricerca, fino alla fine della lista
		if (list->m[index].score > bestScore) {				// Se la mossa ha un punteggio maggiore di quella impostata in precedenza
			bestScore = list->m[index].score;				// aggiorna il punteggio massimo e
			bestNum = index;								// tiene traccia dell'indice in lista della mossa che ha il miglior punteggio
		}

	assert(bestNum >= 0 && bestNum < list->nmoves);			// Controllo validita' indice elemento trovato in lista
	assert(bestNum >= moveNum);								// Controllo relativo tra gli indici

	temp = list->m[moveNum];
	list->m[moveNum] = list->m[bestNum];
	list->m[bestNum] = temp;								// Scambia la prossima mossa che verra' estratta dalla lista nella ricerca con la mossa che ha il miglior punteggio

}


//
// Routine di ricerca Alpha Beta fino a una posizione "tranquilla"
// (cioe' l'ultima mossa non e' di cattura) (e non applica mosse nulle)
// E' simile ad AlhaBeta ma non ha in input la profondita' di ricerca
// e il flag per attivare la ricerca con mosse nulle
//
//
// INPUT:	alpha			valore attuale di alfa
//			beta			valore attuale di beta
// OUTPUT:					Punteggio della posizione iniziale (in centesimi di pedone
//							visto dalla parte che deve muovere

int Quiescence(int alpha,int beta) {

	MList list;							// Lista delle mosse di cattura ammissibili nella posizione
	int Score;							// Punteggio statico della posizione
	int OldAlpha;						// Valore di ingresso di alpha
	int MoveNum;						// Indice il lista mosse di cattura

	char moveBuffer[MAXLMOVE];
	DLL_Result esito;
	
	assert((*pIsPosOk)());									// Controllo coerenza della posizione

	CheckUp();                                              // controlla se forzare fine ricerca
	
	info->nodes++;											// Aggiorna il numero dei nodi visitati nella ricerca
	*info->plNodes = (long)info->nodes;						// Per la GUI

	if (pIsDraw && (*pIsDraw)())							// Test se patta per ripetizione 3 mosse o per regola delle 50 mosse
		return DRAW_SCORE;									// 0 = punteggio di patta

	if (EngineOptions->Quiescence == FALSE)
		return (*pEval)(EngineOptions->RawEval);			// Non c'e' ricerca con quiescenza

	if ((*pGetPly)() > MAXDEPTH - 1)						// Per sicurezza controlla che la profondita' attuale non comporti uno sforamento per una successiva ricerca
		return (*pEval)(EngineOptions->RawEval);			// In tal caso esce con la valutazione statica della posizione raggiunta

	Score = (*pEval)(EngineOptions->RawEval);				// Valuta staticamente la posizione

	if (Score >= beta)
		return beta;										// beta cutoff sulla valutazione statica

	if (Score > alpha)
		alpha = Score;										// Aggiorna il massimo punteggio utilizzando la valutazione statica

	(*pGenMoveAllowed)(&list,TRUE);							// Genera la lista delle mosse di cattura ammissibili nella posizione

	GenStrMoves(&list);										// Genera stringhe mosse nel formato ZoG

	OldAlpha = alpha;										// Copia il valore di alpha in input della funzione

	for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++) {	// Loop per tutte le mosse ammissibili

		PickNextMove(MoveNum,&list);						// Ordina le mosse in modo che la mossa di posto MoveNum sia quella di punteggio maggiore (move ordering)
		
		strcpy(moveBuffer,list.m[MoveNum].str_moves);		// Copia la stringa della mossa nel formato Zog in moveBuffer

		esito = DoMove(moveBuffer);							// Esegue la mossa

		assert(esito == DLL_OK);						// Controllo se mossa corretta

		Score = -Quiescence(-beta,-alpha);				// Si valuta il punteggio della posizione raggiunta con la mossa, in maniera ricorsiva (ricerca alpha-beta appunto)

		(*pTakeBack)();									// Si torna indietro alla posizione in input
			
		if (info->stopped == TRUE && PvArray.nel != 0)		// Test se interrompere la ricerca per tempo scaduto
			return 0;									// Non ha importanza

		if (Score > alpha) {							// Se il punteggio batte alpha

			if (Score >= beta) {						// ma non si e' mantenuto inferiore a beta
					
				if (MoveNum == 1)
					info->fhf++;						// Se e' la prima posizione esaminata e da' subito fail-high incrementa il counter di fail-high-first

				info->fh++;								// Contatore fail-high (cosi' tecnicamente si indica la "potatura" relativa a beta)
														// A questo punto il rapporto fhf/fh (compreso tra 0 e 1) mi da' un'idea di quanto buono e' l'ordinamento delle mosse
														// rispetto alla "potatura" dell'albero esplorato
					
				return beta;							// cut-off al valore di beta
			}
							
			alpha = Score;								// IL punteggio e' maggiore di alpha e minore di beta percio' aggiorna la soglia minima di accettazione
		}
	}

	assert(alpha >= OldAlpha);

	return alpha;											// Punteggio valutato della posizione (sara' sicuramente tra alpha e beta estremi inclusi)
}

//
// Routine di ricerca Alpha Beta
//
// INPUT:	alpha			limite inferiore del punteggio
//			beta			limite superiore del punteggio
//			depth			profondita' di ricerca richiesta
//			DoNull			permesso se utilizzare la ricerca nulla (FALSE/TRUE)
// OUTPUT:					Punteggio della posizione in input (in centesimi di pedone)
//							visto dalla parte che deve muovere

int AlphaBeta(int alpha,int beta,int depth,int DoNull) {

	MList list;							// Lista delle mosse ammissibili nella posizione
	int MoveNum;						// Indice il lista mosse
	int OldAlpha;						// Copia per il valore in input di alpha
	CompMove BestMove;					// Se la mossa corrente nella posizione fornisce un punteggio che supera alpha ed e' inferiore a beta,
										// essa rappresenta la mossa migliore; questa mossa verra' successivamente memorizzata nella variante principale
	int Score;							// Punteggio della posizione generata dalla mossa, ottenuto ricorsivamente.
	int BestScore;						// Migliore punteggio ottenuto dai nodi al livello successivo di profondita'
	CompMove PvMove;					// Eventuale mossa della variante principale memorizzata per la posizione in precedenza
	int extraDepth;						// = TRUE se nella posizione in input il Re della parte che deve muovere e' in scacco

	char moveBuffer[MAXLMOVE];
	DLL_Result esito;

	assert((*pIsPosOk)());				// Controllo coerenza della posizione

	if (depth <= 0)						// Il < e' per sicurezza
		return Quiescence(alpha,beta);	// Se raggiunta la profondita' richiesta passa alla ricerca di quiescenza (valutazioni statiche + ricerca posizione "calma" senza mosse di cattura

	CheckUp();                                              // controlla se forzare fine ricerca
	
	info->nodes++;											// Aggiorna il numero dei nodi visitati nella ricerca
	*info->plNodes = (long)info->nodes;						// Per la GUI

	if ( (pIsDraw && (*pIsDraw)()) && (*pGetPly)() )		// Test se posizione pari per ripetizione  o per regola delle 50 mosse)  0 = punteggio di patta  
		return DRAW_SCORE;									// A livello di root (primo loop iterative deepening) si va' comunque oltre in modo da caricare almeno la prima mossa
															//  della variante principale

	if ((*pGetPly)() > MAXDEPTH - 1)				// Per sicurezza controlla che la profondita' attuale non comporti uno sforamento per una successiva ricerca
		return (*pEval)(EngineOptions->RawEval);	// In tal caso esce con la valutazione statica della posizione raggiunta

	extraDepth = pExtraDepth ? (*pExtraDepth)() : FALSE;		// Valuta se viene richiesto un aumento di profondita'

	if (extraDepth)
		depth++;											// Se e' in scacco setta una profondita' maggiorata (in modo da parare combinazioni di scacchi che portano a posizioni negative

	// Ricerca nella hashtable se la posizione e' gia' stata analizzata

	if (EngineOptions->Hash)												// Se gestione attiva
		if (ProbeHashEntry((*pGetHashKey)(),&PvMove,&Score,alpha,beta,depth)) {		// Se il punteggio e' recuperabile dalla hash
		
			HashTable.cut++;					// Aggiorna il counter 

			return Score;						// Non c'e' bisogno di continuare la ricerca: il punteggio deriva dalla hashtable
		}

	// Gestione della mossa nulla (Null Move)

	if (DoNull &&  depth >=4 && (*pCanDoNull)()) {	// Condizioni per effettuare la mossa nulla:
													// a) Il flag in input DoNull deve essere a TRUE (cio' avviene se AlphaBeta non e' chiamata avendo una ricerca Null Move gia' attiva)
													// b) La parte che muove non puo' essere sotto scacco
													// c) La parte che muove deve avere almeno una Torre o Regina (per limitari gli effetti deleteri del ZugZwang con la Null Move)
													// d) La profondita' richiesta deve essere maggiore di 3 poiche' la ricerca con la Null Move viene fatta fino a una profondita'
													//    inferiore a 4 rispetto all'originale
													// e) Non si deve essere a livello di root (per fare in modo di evitare l'eventuale uscita non avendo caricato la prima mossa
													//    della variante principale
	
		(*pMakeNullMove)();										// Esegue la mossa nulla
		Score = -AlphaBeta(-beta,-beta + 1,depth - 4,FALSE);	// Richiama AlphaBeta disattivando la ricerca con Null Move, a una profondita' inferiore di 4 dell'originale
																// e con una finestra di ampiezza una unita' cioe' molto piccola (vedere la teoria della Null Move)
		(*pTakeNullMove)();										// Torna indietro dalla mossa nulla

		if (info->stopped == TRUE)								// Test eventuale prenotazione d'uscita dalla ricerca
			return 0;

		if (Score >= beta && abs(Score) < ISMATE) {				// Eventuale cut-off con il valore restituito dalla ricerca Null Move (solo se il punteggio non e' relativo ad una posizione di matto)
		
			info->nullCut++;									// Aggiorna counter beta cutoff per null search

			return beta;
		}
	}

	BestScore = LOSS_SCORE;									// Default punteggio massimo

	(*pGenMoveAllowed)(&list,FALSE);						// Genera la lista delle mosse ammissibili nella posizione

	OldAlpha = alpha;										// Copia il valore di alpha in input della funzione

	if (pGetPvScore)										// Se gestione attiva
		if (ProbePvMove((*pGetHashKey)(),&PvMove))				// Se nella tabella hash e' presente una mossa della variante principale relativa alla posizione
			for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++)
				if (CompareMoves(&list.m[MoveNum],&PvMove)) {	// e la mossa e' nella lista delle mosse da provare
			
					list.m[MoveNum].score = (*pGetPvScore)();	// Sovrascrive il punteggio della mossa con quello maggiore rispetto alle altre tecniche di assegnazione del punteggio
																// A questo punto sara' sicuramente QUESTA la prima mossa che verra' prelevata dalla lista con PickNextMove()
					break;										// Esce dalla ricerca
				}

	GenStrMoves(&list);										// Genera stringhe mosse nel formato ZoG

	for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++) {	// Loop per tutte le mosse ammissibili

		PickNextMove(MoveNum,&list);						// Ordina le mosse in modo che la mossa di posto MoveNum sia quella di punteggio maggiore (move ordering)
	
		strcpy(moveBuffer,list.m[MoveNum].str_moves);		// Copia la stringa della mossa nel formato Zog in moveBuffer

		esito = DoMove(moveBuffer);							// Esegue la mossa

		assert(esito == DLL_OK);							// Controllo se mossa corretta

		Score = -AlphaBeta(-beta,-alpha,depth-1,(pMakeNullMove && pTakeNullMove) ? EngineOptions->NullMove : FALSE);	// Si valuta il punteggio della posizione raggiunta con la mossa, in maniera ricorsiva (ricerca alpha-beta appunto)

		(*pTakeBack)();									// Si torna indietro alla posizione in input

		if (info->stopped == TRUE && PvArray.nel)		// Test se interrompere la ricerca per tempo scaduto
			return 0;									// Non ha importanza

		if (Score > BestScore) {						// Test se punteggio migliore del precedente

			BestScore = Score;								// Aggiorna punteggio migliore
			BestMove = list.m[MoveNum];						// Memorizza in BestMove la mossa appena effettuata che risulta essere la migliore trovata

			if (Score > alpha) {							// Se il punteggio ora migliorato batte alpha

				if (Score >= beta) {						// ma non si e' mantenuto inferiore a beta
					
					if (MoveNum == 1)
						info->fhf++;						// Se e' la prima posizione esaminata e da' subito fail-high incrementa il counter di fail-high-first

					info->fh++;								// Contatore fail-high (cosi' tecnicamente si indica la "potatura" relativa a beta)
															// A questo punto il rapporto fhf/fh (compreso tra 0 e 1) mi da' un'idea di quanto buono e' l'ordinamento delle mosse
															// rispetto alla "potatura" dell'albero esplorato

				// Euristica relativa alla mossa che ha provocato un beta cutoff

					if (!(list.m[MoveNum].flags & CAPTURE) && pSetKillers)
						(*pSetKillers)(&list.m[MoveNum]);	// Euristica sulla mossa di beta cutoff

					StoreHashEntry((*pGetHashKey)(), BestMove, beta, HFBETA, depth);	// Memorizza la posizione nella hash dandogli il punteggio di beta e dichiarandolo di tipo HFBETA
					
					return beta;							// cut-off al valore di beta
				}
							
			alpha = Score;								// Il punteggio e' maggiore di alpha percio' aggiorna la soglia minima di accettazione

			// Euristica relativa alla mossa che ha migliorato alpha

			if (!(list.m[MoveNum].flags & CAPTURE) && pSetHistory)	// Se la mossa non e' di cattura...
				(*pSetHistory)(list.m[MoveNum],depth);	// Euristica su mossa che batte alfa	
			}
		}
	}

	// Controllo scaccomatto/stallo

	if ((*pCheckLoss)())
		return LOSS_SCORE + (*pGetPly)();				// si ha lo scacco matto e si esce con un punteggio negativissimo contenente in qualmodo informazione sulla distanza di semimosse dal matto

	if ((*pCheckDraw)())
		return DRAW_SCORE;								// Se il Re non e' in scacco e' Stallo: si esce col punteggio di parita'

	assert(alpha>=OldAlpha);

	if (alpha != OldAlpha)					
		StoreHashEntry((*pGetHashKey)(),BestMove,BestScore,HFEXACT,depth);	// Se il punteggio ha superato l'alfa di ingresso memorizza in hashtable con il punteggio BestScore (che vale alpha) e rappresenta il punteggio esatto in out
	else
		StoreHashEntry((*pGetHashKey)(),BestMove,alpha,HFALPHA,depth);		// Altrimenti memorizza dando come punteggio l'alfa di ingresso e dichiarando l'entry di tipo HFALPHA

	return alpha;											// Punteggio valutato della posizione (sara' sicuramente tra alpha e beta estremi inclusi)
}


//
// Questa funzione realizza una ricerca della mossa migliore
// con iterative deepening

void SearchPosition(void) {

	int currentDepth;				// profondita' corrente nell'iterative deepening
	int bestScore;					// punteggio valutato per la posizione pos
	CompMove bestMove;				// la prima mossa della variante principale ovvero la mossa migliore a livello di root
									// ovvero nella posizione iniziale *pos
	int pvNum;						// Indice nella variante principale
	MList list;

	bestMove.nbmoves = 0;			// Per poter riconoscere se e' stato caricato almeno una volta
	
	PvArray.nel = 0;				// Inizializzazione variante principale

	// Inizializza le informazioni globali della hashtable (NOTA: il contenuto della hash NON viene cancellato!)

	HashTable.overWrite = 0;
	HashTable.hit = 0;
	HashTable.cut = 0;

	// Inizializza il database della ricerca

	info->stopped = 0;
	info->nodes = 0ULL;
	info->fh = 0;
	info->fhf = 0;
	info->nullCut = 0;

	(*pInitSearch)();				// Reset variabili della ricerca

	if (EngineOptions->Book && pGetBookMove)				// Se libreria delle aperture attiva
		bestMove = (*pGetBookMove)();						// Cerca la mossa nella libreria
	
	if (bestMove.nbmoves) {									// Se mossa non trovata in libreria esegue la ricerca
	
		list.nmoves = 1;
		list.m[0] = bestMove;

		GenStrMoves(&list);									// Genera stringa mossa in formato ZoG

		strcpy(info->bMove,list.m[0].str_moves);			// Copia per la GUI
		strcpy(info->currentMove,list.m[0].str_moves);

		*info->plScore = 0;
		*info->plDepth = 1;

		PvArray.nel = 1;
		PvArray.Pv[0] = bestMove;
	}
	else
		for ( currentDepth = 1; currentDepth <= info->depth; currentDepth++) {	// ricerche a profondita' crescenti da 1 fino alla massima richiesta

			switch (EngineOptions->SearchType) {
			
				case	ALPHABETA:	bestScore = AlphaBeta(LOSS_SCORE,WIN_SCORE,currentDepth,(pMakeNullMove && pTakeNullMove) ? EngineOptions->NullMove : FALSE);	// Avvio ricerca con Alpha= -infinito e Beta= +infinito fino alla profondita' currentDepth
									break;
				case	MINMAX:		bestScore = ((*pGetSide)() == MAXP) ? Max1(currentDepth) : Min1(currentDepth);	// MinMax
									break;
				case	NEGAMAX:	bestScore = NegaMax(currentDepth);												// NegaMax
									break;
			}

			if (info->stopped == TRUE && bestMove.nbmoves)		// Se tempo scaduto avendo caricato qualcosa in bestmove
				break;											// Rimarranno buoni i risultati ottenuti alla profondita' precedente

			GetPvLine(currentDepth);							// Ricava il numero di mosse della variante principale

			if (PvArray.nel)
				bestMove = PvArray.Pv[0];						// Ricava la prima mossa della variante principale

			list.nmoves = 1;
			list.m[0] = bestMove;

			GenStrMoves(&list);									// Genera stringa mossa in formato ZoG

			strcpy(info->bMove,list.m[0].str_moves);			// Copia per la GUI

			list.nmoves = 1;

			for (pvNum = 0; pvNum < PvArray.nel; pvNum++) {

				list.m[0] = PvArray.Pv[pvNum];

				GenStrMoves(&list);								// Genera stringa mossa in formato ZoG
			
				if (pvNum == 0)
					strcpy(info->currentMove,list.m[0].str_moves);
				else {
					strcat(info->currentMove," ");
					strcat(info->currentMove,list.m[0].str_moves);
				}
			}

			strcat(info->currentMove,"\n");

			*info->plScore = bestScore;
			*info->plDepth = currentDepth;

			if (EngineOptions->SearchType == MINMAX)			// Se tipo ricerca MinMax
				if ((*pGetSide)() == MINP)						// Se muove il Nero
					bestScore = - bestScore;					// In modo che il punteggio venga visualizzato correttamente dal protocollo (il punteggio si deve riferire a chi ha la mossa)
	
			DoLog("info score cp %d depth %d nodes %llu time %d ",bestScore,currentDepth,info->nodes,GetTimeMs() - info->starttime);			// Protocollo UCI (la stringa si concatena con la stampa "pv" successiva)

			if (pPrintMove) {
				DoLog("pv");											// Variante principale

				for (pvNum = 0; pvNum < PvArray.nel; pvNum++)
					DoLog(" %s",(*pPrintMove)(&PvArray.Pv[pvNum]));		// Stampa le mosse della variante principale

				DoLog("\n");
			}

			if (UciMode && pPrintMove) {

				printf("info score cp %d depth %d nodes %llu time %d ",bestScore,currentDepth,info->nodes,GetTimeMs() - info->starttime);			// Protocollo UCI (la stringa si concatena con la stampa "pv" successiva)

				printf("pv");											// Variante principale

				for (pvNum = 0; pvNum < PvArray.nel; pvNum++)
					printf(" %s",(*pPrintMove)(&PvArray.Pv[pvNum]));	// Stampa le mosse della variante principale

				printf("\n");
			}
		}

		if (UciMode && pPrintMove)
			printf("bestmove %s\n",(*pPrintMove)(&bestMove));
}
