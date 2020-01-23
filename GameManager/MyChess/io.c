#include "defs.h"

//
// Funzione che crea una stringa corrispondente alla notazione algebrica di una casella (es: 'a1' 'h5' etc..)
//
// INPUT:	sq		Casella della scacchiera estesa
// OUTPUT:			pointer alla stringa
//

char *PrSq(int sq) {

	static char SqStr[3];		// La stringa in uscita (static poiche' deve esistere anche usciti dalla funzione)
	int	file;					// Colonna della casella
	int rank;					// Traversa della casella

	file = FilesBrd[sq];		// Ricava colonna (0-7)
	rank = RanksBrd[sq];		// Ricava traversa (0-7)
#if defined(_MSC_VER) || defined(__MINGW32__)
	sprintf_s(SqStr,3,"%c%c",'a' + file, '1' + rank);		// Deposita in SqStr i 2 caratteri di colonna,riga e NULL (fine stringa) 
#else
	sprintf(SqStr,"%c%c",'a' + file, '1' + rank);			// Deposita in SqStr i 2 caratteri di colonna,riga e NULL (fine stringa) 
#endif
	return SqStr;
}


//
// Funzione che crea la stringa corrispondente ad una mossa ( es: b2c3  g2g4 d2d1q (quest'ultima con promozione a Regina) )
//
// INPUT:	move		mossa
// OUTPUT:				stringa della mossa
//

char *PrMove(int move) {

	static char MvStr[6];		// Stringa in out 5 caratteri + NULL

	int ff;						// Colonna casella di partenza
	int	rf;						// Traversa casella di partenza
	int	ft;						// Colonna casella di arrivo
	int	rt;						// Traversa colonna di arrivo
	int	promoted;				// Codice pezzo promosso (= 0 se no promozione)
	char pchar = '\0';			// Conterra' il carattere corrispondente all'eventuale pezzo promosso (default stringa vuota)

	ff = FilesBrd[FROMSQ(move)];
	rf = RanksBrd[FROMSQ(move)];
	ft = FilesBrd[TOSQ(move)];
	rt = RanksBrd[TOSQ(move)];		// Ricava informazioni dalla mossa codificata

	promoted = PROMOTED(move);		// Ricava codice pezzo promosso (= 0 se non pezzo promosso)

	if (promoted) {					// Se c'e' una promozione
		pchar = 'q';				// Per default promuove a Regina
		if (IsKn(promoted))
			pchar = 'n';			// Test se e' un Cavallo
		if (IsRQ(promoted)) {		// Se il pezzo promosso e' una Torre o una Regina
			if (!IsBQ(promoted))	// e non e' ne' un Alfiere ne' una Regina
				pchar = 'r';		// allora il pezzo promosso e' una Torre	
		}
		else
			if (IsBQ(promoted))		// Ma se non e' ne Torre ne' Regina ma e' Alfiere o Regina
				pchar = 'b';		// allora il pezzo promosso e' un Alfiere
	}
#if defined(_MSC_VER) || defined(__MINGW32__)
	sprintf_s(MvStr,6,"%c%c%c%c%c",'a' + ff, '1' + rf, 'a' + ft, '1' + rt, pchar);
#else
	sprintf(MvStr,"%c%c%c%c%c",'a' + ff, '1' + rf, 'a' + ft, '1' + rt, pchar);
#endif
	return MvStr;
}

//
// Questa funzione decodifica la stringa in input
// come mossa lecita relativamente alla posizione attuale
// (e alla parte che deve muovere naturalmente!)
//
// INPUT:	ptrChar		pointer alla mossa da decodificare
// OUTPUT:	NOMOVE		Mossa non ammissibile (=0)
//						Altrimenti la mossa e' ammissibile
//						e il valore di ritorno e' la mossa in input codificata in un intero
//						NOTA: la mossa dichiarata ammissibile potrebbe pero' essere illegale
//						se la parte che ha fatto la mossa e' sotto scacco. Questo puo'
//						essere controllato valutando il valore di ritorno della MakeMoveV sulla
//						mossa in questione. Se il valore di ritorno non e' TRUE la mossa
//						e' illegale e la MakeMoveV non la esegue.

int ParseMove(char *ptrChar) {

	int	from;			// Casella di partenza
	int	to;				// Casella di arrivo
	S_MOVELIST	list;	// Lista delle mosse ammesse nella posizione
	int MoveNum;		// Indice in lista mosse ammesse
	int PromPce;		// Codice pezzo promosso
	int Move;			// Appoggio mossa corrente nella lista

	// Controllo correttezza caselle di partenza e arrivo

	if (ptrChar[0] < 'a' || ptrChar[0] > 'h') return NOMOVE;		// Errore in decodifica colonna casella di partenza
	if (ptrChar[1] < '1' || ptrChar[1] > '8') return NOMOVE;		// Errore in decodifica traversa casella di partenza
	if (ptrChar[2] < 'a' || ptrChar[2] > 'h') return NOMOVE;		// Errore in decodifica colonna casella di arrivo
	if (ptrChar[3] < '1' || ptrChar[3] > '8') return NOMOVE;		// Errore in decodifica traversa casella di arrivo

	// Decodifica le caselle di partenza e arrivo

	from = FR2SQ(ptrChar[0] - 'a',ptrChar[1] - '1');
	to = FR2SQ(ptrChar[2] - 'a',ptrChar[3] - '1');

	//DoLog("ptrChar: %s  from: %d to: %d\n",ptrChar,from,to);		// Stampa info mossa

	assert(SqOnBoard(from) && SqOnBoard(to));						// Ulteriore controllo che le caselle appartengano alla scacchiera reale

	// Ora si ricerca una mossa legale che abbia le caselle di partenza e arrivo e eventuale pezzo promosso
	// uguali alla mossa in input:	questo ci assicurera' che la mossa in input oltre ad essere sintatticamente corretta
	// e' anche una mossa legale per la posizione impostata

	GenerateAllMoves(pos,&list);									// Genera tutte le mosse possibili nella posizione

	for (MoveNum = 0; MoveNum < list.count; MoveNum++) {			// Loop su tutte le mosse della lista
	
		Move = list.moves[MoveNum].move;							// Ricava la mossa dalla lista
		
		if (FROMSQ(Move) == from && TOSQ(Move) == to) {				// Se casella di partenza e arrivo corrispondono
		
			PromPce = PROMOTED(Move);								// Ricava codice pezzo promosso

			if (PromPce == EMPTY)									// Se non c'e' una promozione la mossa puo' essere dichiarata legale
				return Move;										// Ritorna la mossa codificata in un intero
				
			// C'e' una promozione: per dichiarare la mossa legale bisogna verificare
			// che anche la mossa in ingresso dichiari la promozione e i 2 pezzi promossi vanno confrontati

			if (IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4]=='r')	// Il pezzo promosso e' una Torre ed e' lo stesso dichiarato nella mossa in input
				return Move;										// Ritorna la mossa codificata in un intero
			if (!IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4]=='b')	// Il pezzo promosso e' un Alfiere ed e' lo stesso dichiarato nella mossa in input
				return Move;										// Ritorna la mossa codificata in un intero
			if (IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4]=='q')	// Il pezzo promosso e' una Regina ed e' lo stesso dichiarato nella mossa in input
				return Move;										// Ritorna la mossa codificata in un intero
			if (IsKn(PromPce) && ptrChar[4]=='n')					// Il pezzo promosso e' un Cavallo ed e' lo stesso dichiarato nella mossa in input
				return Move;										// Ritorna la mossa codificata in un intero
			}
		}

	return NOMOVE;													// La mossa e' illegale nella posizione considerata
}


//
// Funzione che stampa una lista di mosse
//
// INPUT:	list	pointer alla lista delle mosse
//

void PrintMoveList(S_MOVELIST *list) {

	int nm;				// Indice mossa corrente nella lista

	DoLog("MoveList: %d\n\n",list->count);

	for (nm = 0; nm < list->count; nm++)
		DoLog("Move: %d > %s  (score: %d)\n",nm + 1,PrMove(list->moves[nm].move),list->moves[nm].score);

	DoLog("\nMoveList Total %d Moves\n\n",list->count);
}
