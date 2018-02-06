#include "defs.h"

// Macro che costruisce una mossa a partire dalle componenti
//
//	f	casella from
//	t	casella	to
//	ca	codice pezzo catturato
//	pro	codice pezzo promosso
//	fl	flag vari

#define	MOVE(f,t,ca,pro,fl)	( (f) | ( (t) << 7 ) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl) )


//
// Macro che fornisce 1 se la casella sq della scacchiera estesa non appartiene a quella reale
//

#define	SQOFFBOARD(sq)	( FilesBrd[(sq)] == OFFBOARD )		// Si poteva usare anche RanksBrd[sq]

//
// Vettore contenente 2 liste con i codici dei pezzi che minacciano a distanza.
// La prima lista e' quella del Bianco, l'altra quella del Nero. Ogni lista termina con uno zero
//

int LoopSlidePce[] = { wB, wR, wQ, 0, bB, bR, bQ, 0 };

//
// Vettore contenente 2 liste con i codici dei pezzi che non minacciano a distanza.
// La prima lista e' quella del Bianco, l'altra quella del Nero. Ogni lista termina con uno zero
// NOTA: il pedone non viene inserito poiche' e' stato gestito in maniera peculiare
//

int LoopNonSlidePce[] = { wN, wK, 0, bN, bK, 0 };

//
// Vettore con i 2 indici di inizio lista (Per il Bianco e per il Nero) nel vettore LoopSlidePieces[]
//

int LoopSlideIndex[] = { 0, 4 };

//
// Vettore con i 2 indici di inizio lista (Per il Bianco e per il Nero) nel vettore LoopNonSlidePieces[]
//

int LoopNonSlideIndex[] = { 0, 3 };

//
// Matrice con i displacement dei movimenti dei vari pezzi
// Il primo indice indica il codice del pezzo
// e in ogni riga sono presenti i displacement relativi ai movimenti nelle varie direzioni
// Il displacement = 0 significa che non e' significativo
// Ricordiamo che il Re, il Cavallo e la Regina hanno 8 direzioni
// La Torre e l'Alfiere solo 4
// NOTA: il pedone ha una gestione speciale e per esso la tabella non viene utilizzata

int PceDir[][8] = {

	{ 0, 0, 0, 0, 0, 0, 0, 0 },				// EMPTY
	{ 0, 0, 0, 0, 0, 0, 0, 0 },				// Pedone bianco
	{ -21, -19, -12, -8, 8, 12, 19, 21 },	// Cavallo bianco
	{ -11, -9, 9, 11, 0, 0, 0, 0 },			// Alfiere bianco
	{ -10, -1, 1, 10, 0, 0, 0, 0 },			// Torre bianca
	{ -11, -10, -9, -1, 1, 9, 10, 11 },		// Regina bianca
	{ -11, -10, -9, -1, 1, 9, 10, 11 },		// Re bianco
	{ 0, 0, 0, 0, 0, 0, 0, 0 },				// Pedone nero
	{ -21, -19, -12, -8, 8, 12, 19, 21 },	// Cavallo nero
	{ -11, -9, 9, 11, 0, 0, 0, 0 },			// Alfiere nero
	{ -10, -1, 1, 10, 0, 0, 0, 0 },			// Torre nera
	{ -11, -10, -9, -1, 1, 9, 10, 11 },		// Regina nera
	{ -11, -10, -9, -1, 1, 9, 10, 11 }		// Re nero
};


//
// Vettore che specifica quante direzioni di movimento ha ciascun pezzo
// Serve per sapere quanti elementi di direzione utilizzare nella tabella precedente
// NOTA:	EMPTY e pedone non vengono utilizzati
//

int NumDir[] = { 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8 };


//
// Punteggio assegnato alla vittima di una presa in base al tipo di pezzo
// Come si vede il punteggio e' crescente in base alla forza del pezzo
// Chiaramente la posizione corrispondente a EMPTY=0 non verranno utilizzata

int VictimScore[] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };


//
// Matrice indicizzata con i codici pezzo di vittima e attaccante in una presa
// che andra' inizializzata con il punteggi della corrsipondente presa
// seguendo la regola MvvLVA (Most valuable victim - Least valuable attacker)
// Cioe' la presa ha un punteggio maggiore tanto piu' forte e' il pezzo catturato
// e tanto piu' debole e' il pezzo che cattura; la presa di punteggio maggiore
// e' percio'  Pedone x Regina
//

int MvvLvaScores[13][13];


//
// Routine per inizializzare la matrice MvvLva
// in base alla regola Most valuable victim - Least valuable attacker
// NOTA: Vengono anche inizializzate le locazioni corrispondenti
// a coppie attacker/victim dello stesso colore, ma chiaramente queste
// combinazioni non hanno significato. Le locazioni corrispondenti al Re
// come vittima non verranno mai utilizzate.
//
// Come si vede il valore assegnato e' la somma di 2 valori:
// VictimScore[Victim] che cresce al crescere della forza della vittima e
// 6 - VictimScore[Attacker] / 100 che cala al crescere delle forza del pezzo che cattura:
// Infatti tale valore parte da 5 per il Pedone e scende a 1 per la Regina e a 0 per il Re

void InitMvvLva(void) {

	int Attacker;		// Codice pezzo che ha l'attacco
	int Victim;			// Codice pezzo che viene catturato

	for (Attacker = wP; Attacker <= bK; Attacker++)		// Loop pezzi di attacco
		for (Victim = wP; Victim <= bK; Victim++)		// Loop pezzi catturati
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - VictimScore[Attacker] / 100;
#if 0
	for (Attacker = wP; Attacker <= bK; Attacker++)		// Loop pezzi di attacco
		for (Victim = wP; Victim <= bK; Victim++)		// Loop pezzi catturati
			DoLog("%c x %c = %d\n", PceChar[Attacker],PceChar[Victim],MvvLvaScores[Victim][Attacker]);			// Stampa matrice
#endif
}


//
// Questa funzione aggiunge una mossa non di cattura alla lista delle mosse
//
// INPUT:	pos			pointer alla posizione dalla quale si effettua la mossa
//			move		la mossa inserire in lista
//			list		pointer alla lista

void AddQuietMove(S_BOARD *pos,int move,S_MOVELIST *list) {

	assert(SqOnBoard(FROMSQ(move)));				// Controllo validita' casella di partenza della mossa
	assert(SqOnBoard(TOSQ(move)));					// Controllo validita' casella di arrivo della mossa

	list->moves[list->count].move = move;

	// Euristica con le 2 mosse killers

	if (pos->searchKillers[0][pos->ply] == move)
		list->moves[list->count++].score = 900000;	// Se e' l'ultima (in ordine temporale) mossa killer rilevata da punteggio 900000 (ricordiamo che le mosse di cattura hanno punteggi superiori al milione)
	else
	if (pos->searchKillers[1][pos->ply] == move)
		list->moves[list->count++].score = 800000;	// Se e' la penultima (in ordine temporale) mossa killer rilevata da punteggio 800000 (ricordiamo che le mosse di cattura hanno punteggi superiori al milione)
	else

		list->moves[list->count++].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];		// Mossa di non cattura che non e' una killer move : utilizza l'euristica legata alle mosse che "battono" alpha
}

//
// Questa funzione aggiunge una mossa di cattura alla lista delle mosse
//
// INPUT:	pos			pointer alla posizione dalla quale si effettua la mossa
//			move		la mossa inserire in lista
//			list		pointer alla lista

void AddCaptureMove(S_BOARD *pos,int move,S_MOVELIST *list) {

	assert(SqOnBoard(FROMSQ(move)));				// Controllo validita' casella di partenza della mossa
	assert(SqOnBoard(TOSQ(move)));					// Controllo validita' casella di arrivo della mossa
	assert(PieceValid(CAPTURED(move)));				// Controllo validita' codice pezzo catturato

	list->moves[list->count].move = move;
	list->moves[list->count++].score = MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1000000;	// Vittima=pezzo catturato Attacker=pezzo che si sta muovendo (+ 1000000 per lasciare spazio ai punteggi di non cattura che devono essere inferiori)
}

//
// Questa funzione aggiunge una mossa di cattura enpassant alla lista delle mosse
//
// INPUT:	pos			pointer alla posizione dalla quale si effettua la mossa
//			move		la mossa inserire in lista
//			list		pointer alla lista

void AddEnPassantMove(S_BOARD *pos,int move,S_MOVELIST *list) {

	assert(SqOnBoard(FROMSQ(move)));				// Controllo validita' casella di partenza della mossa
	assert(SqOnBoard(TOSQ(move)));					// Controllo validita' casella di arrivo della mossa

	list->moves[list->count].move = move;
	list->moves[list->count++].score = MvvLvaScores[wP][bP] + 1000000;			// In pratica 100 + 6 - 100/100 = 105 (Attacker=Pedone e Victim=Pedone)  (+ 1000000 per lasciare spazio ai punteggi di non cattura che devono essere inferiori)
}

//
// Questa funzione aggiunge una mossa di cattura da parte di un pedone bianco
//
// INPUT:	pos		pointer alla posizione
//			from	casella di partenza
//			to		casella di arrivo
//			cap		codice del pezzo catturato
//			list	pointer alla lista in cui depositare la mossa
//

void AddWhitePawnCapMove(S_BOARD *pos,int from,int to,int cap,S_MOVELIST *list) {

	assert(PieceValidEmpty(cap));							// Controllo che il codice pezzo sia valido (Considera buono anche EMPTY) ??
	assert(SqOnBoard(from));								// Controllo che la casella di partenza sia valida
	assert(SqOnBoard(to));									// Controllo che la casella di arrivo sia valida

	if (RanksBrd[from] == RANK_7) {							// Se parte dalla settima traversa e' una mossa di promozione
	
		AddCaptureMove(pos,MOVE(from,to,cap,wQ,0),list);	// Inserisce mossa di cattura con promozione a Regina
		AddCaptureMove(pos,MOVE(from,to,cap,wR,0),list);	// Inserisce mossa di cattura con promozione a Torre
		AddCaptureMove(pos,MOVE(from,to,cap,wB,0),list);	// Inserisce mossa di cattura con promozione a Alfiere
		AddCaptureMove(pos,MOVE(from,to,cap,wN,0),list);	// Inserisce mossa di cattura con promozione a Cavallo
	
	}
	else
		AddCaptureMove(pos,MOVE(from,to,cap,EMPTY,0),list);	// In caso contrario inserisce una mossa di cattura senza promozione
}


//
// Questa funzione aggiunge una mossa di cattura da parte di un pedone nero
//
// INPUT:	pos		pointer alla posizione
//			from	casella di partenza
//			to		casella di arrivo
//			cap		codice del pezzo catturato
//			list	pointer alla lista in cui depositare la mossa
//

void AddBlackPawnCapMove(S_BOARD *pos,int from,int to,int cap,S_MOVELIST *list) {
	
	assert(PieceValidEmpty(cap));							// Controllo che il codice pezzo sia valido (Considera buono anche EMPTY) ??
	assert(SqOnBoard(from));								// Controllo che la casella di partenza sia valida
	assert(SqOnBoard(to));									// Controllo che la casella di arrivo sia valida

	if (RanksBrd[from] == RANK_2) {							// Se parte dalla seconda traversa e' una mossa di promozione
	
		AddCaptureMove(pos,MOVE(from,to,cap,bQ,0),list);	// Inserisce mossa di cattura con promozione a Regina
		AddCaptureMove(pos,MOVE(from,to,cap,bR,0),list);	// Inserisce mossa di cattura con promozione a Torre
		AddCaptureMove(pos,MOVE(from,to,cap,bB,0),list);	// Inserisce mossa di cattura con promozione a Alfiere
		AddCaptureMove(pos,MOVE(from,to,cap,bN,0),list);	// Inserisce mossa di cattura con promozione a Cavallo
	
	}
	else
		AddCaptureMove(pos,MOVE(from,to,cap,EMPTY,0),list);	// In caso contrario inserisce una mossa di cattura senza promozione
}


//
// Questa funzione aggiunge una mossa non di cattura da parte di un pedone bianco
//
// INPUT:	pos		pointer alla posizione
//			from	casella di partenza
//			to		casella di arrivo
//			list	pointer alla lista in cui depositare la mossa
//

void AddWhitePawnMove(S_BOARD *pos,int from,int to,S_MOVELIST *list) {
	
	assert(SqOnBoard(from));								// Controllo che la casella di partenza sia valida
	assert(SqOnBoard(to));									// Controllo che la casella di arrivo sia valida

	if (RanksBrd[from] == RANK_7) {							// Se parte dalla settima traversa e' una mossa di promozione
	
		AddQuietMove(pos,MOVE(from,to,EMPTY,wQ,0),list);	// Inserisce mossa non di cattura con promozione a Regina
		AddQuietMove(pos,MOVE(from,to,EMPTY,wR,0),list);	// Inserisce mossa non di cattura con promozione a Torre
		AddQuietMove(pos,MOVE(from,to,EMPTY,wB,0),list);	// Inserisce mossa non di cattura con promozione a Alfiere
		AddQuietMove(pos,MOVE(from,to,EMPTY,wN,0),list);	// Inserisce mossa non di cattura con promozione a Cavallo
	
	}
	else
		AddQuietMove(pos,MOVE(from,to,EMPTY,EMPTY,0),list);	// In caso contrario inserisce una mossa non di cattura senza promozione
}


//
// Questa funzione aggiunge una mossa non di cattura da parte di un pedone nero
//
// INPUT:	pos		pointer alla posizione
//			from	casella di partenza
//			to		casella di arrivo
//			list	pointer alla lista in cui depositare la mossa
//

void AddBlackPawnMove(S_BOARD *pos,int from,int to,S_MOVELIST *list) {
		
	assert(SqOnBoard(from));								// Controllo che la casella di partenza sia valida
	assert(SqOnBoard(to));									// Controllo che la casella di arrivo sia valida

	if (RanksBrd[from] == RANK_2) {							// Se parte dalla seconda traversa e' una mossa di promozione
	
		AddQuietMove(pos,MOVE(from,to,EMPTY,bQ,0),list);	// Inserisce mossa non di cattura con promozione a Regina
		AddQuietMove(pos,MOVE(from,to,EMPTY,bR,0),list);	// Inserisce mossa non di cattura con promozione a Torre
		AddQuietMove(pos,MOVE(from,to,EMPTY,bB,0),list);	// Inserisce mossa non di cattura con promozione a Alfiere
		AddQuietMove(pos,MOVE(from,to,EMPTY,bN,0),list);	// Inserisce mossa non di cattura con promozione a Cavallo
	
	}
	else
		AddQuietMove(pos,MOVE(from,to,EMPTY,EMPTY,0),list);	// In caso contrario inserisce una mossa non di cattura senza promozione
}


//
// Funzione che genera la lista di tutte le mosse possibili in una posizione
//
// INPUT:	pos		pointer alla posizione
//			list	pointer alla lista da generare

void GenerateAllMoves(S_BOARD *pos,S_MOVELIST *list) {

	int pceNum;							// Indice corrente nella lista dei pezzi presenti di un certo tipo
	int sq;								// Casella interessata nella scacchiera estesa
	int pceIndex;						// Indice nella lista dei pezzi che minacciano a distanza e non
	int pce;							// Codice pezzo corrente nella lista
	int dir;							// Direzione corrente
	int t_sq;							// Appoggio per sq

	assert(CheckBoard(pos));			// Controlla che la posizione sia coerente

	list->count = 0;					// Dichiara vuota la lista da generare

	// Gestione del Pedone e dell'arrocco

	if (pos->side == WHITE) {				// Test se deve gestire una mossa del Bianco

		// Gestione del Pedone del Bianco

		for (pceNum = 0; pceNum < pos->pceNum[wP]; pceNum++) {		// Loop per tutti i pedoni bianchi presenti nella posizione

			sq = pos->pList[wP][pceNum];							// Ricava la casella di presenza del pedone bianco da valutare

			assert(SqOnBoard(sq));				// Controllo che la casella non sia invalida

			// Valutazione mossa senza cattura

			if (pos->pieces[sq + 10] == EMPTY) {		// Se la casella di fronte al pedone (traversa successiva) e' vuota
														// la mossa e' possibile
				AddWhitePawnMove(pos,sq,sq + 10,list);	// Aggiunge mossa di non cattura alla lista

				if (RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY)	// Test su mossa di 2 caselle da posizione iniziale
					AddQuietMove(pos,MOVE(sq,sq + 20,					// Non e' certamente una mossa di promozione
						EMPTY, EMPTY, MFLAGPS),list);					// Aggiunge mossa 2 caselle da posizione iniziale
			}

			// Valutazione mossa con cattura non en-passant

			if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK)	// Se su diagonale sinistra c'e' un pezzo del Nero 
				AddWhitePawnCapMove(pos,sq,sq + 9,pos->pieces[sq + 9],list);	// Aggiunge mossa di cattura di pedone alla lista

			if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK)// Se su diagonale destra c'e' un pezzo del Nero 
				AddWhitePawnCapMove(pos,sq,sq + 11,pos->pieces[sq + 11],list);	// Aggiunge mossa di cattura di pedone alla lista

			// Valutazione mossa con cattura en-passant

			if (pos->enPas != NO_SQ) {												// Deve essere attiva la casella en-passant (altrimenti NO_SQ che vale 99 puo' eguagliare sq+11 se sq=88 (casella h7) !!! )
				if (sq + 9 == pos->enPas)											// Diagonale a sinistra
					AddEnPassantMove(pos,MOVE(sq,sq + 9,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant

				if (sq + 11 == pos->enPas)											// Diagonale a destra
					AddEnPassantMove(pos,MOVE(sq,sq + 11,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant
			}
		}
			// Gestione dell'arrocco del Bianco

			if (pos->castlePerm & WKCA)					// Test se e' permesso l'arrocco del Bianco sul lato di Re
				if (pos->pieces[F1] == EMPTY &&
					pos->pieces[G1] == EMPTY)			// In tal caso controlla che le caselle
														// tra Re e Torre in H1 siano vuote
					if (!SqAttacked(E1,BLACK) &&		// Controlla che il Re bianco non sia sotto scacco
						!SqAttacked(F1,BLACK))			// Controlla che la casella di transito del Re non sia minacciata
						AddQuietMove(pos,MOVE(E1,G1,EMPTY,EMPTY,MFLAGCA),list);		// In tal caso l'arrocco e' una mossa ammessa
																					// e aggiunge la mossa di arrocco alla lista
														// NOTA: non si controlla che la casella G1 sia minacciata
														// poiche' comunque eseguita una qualunque mossa si
														// controllera' sempre che il Re non sia sotto scacco
														// prima di dichiara che la mossa eseguita e' legale
			if (pos->castlePerm & WQCA)					// Test se e' permesso l'arrocco del Bianco sul lato di Donna
				if (pos->pieces[D1] == EMPTY &&
					pos->pieces[C1] == EMPTY &&
					pos->pieces[B1] == EMPTY)			// In tal caso controlla che le caselle
														// tra Re e Torre in A1 siano vuote
					if (!SqAttacked(E1,BLACK) &&		// Controlla che il Re bianco non sia sotto scacco
						!SqAttacked(D1,BLACK))			// Controlla che la casella di transito del Re non sia minacciata
						AddQuietMove(pos,MOVE(E1,C1,EMPTY,EMPTY,MFLAGCA),list);		// In tal caso l'arrocco e' una mossa ammessa
																					// e aggiunge la mossa di arrocco alla lista
														// NOTA: non si controlla che la casella C1 sia minacciata
														// poiche' comunque eseguita una qualunque mossa si
														// controllera' sempre che il Re non sia sotto scacco
														// prima di dichiara che la mossa eseguita e' legale
	}
	else {

		// Gestione del Pedone del Nero

		for (pceNum = 0; pceNum < pos->pceNum[bP]; pceNum++) {		// loop per tutti i pedoni neri presenti nella posizione

			sq = pos->pList[bP][pceNum];							// ricava la casella di presenza del pedone nero da valutare

			assert(SqOnBoard(sq));				// Controllo che la casella non sia invalida

			// Valutazione mossa senza cattura

			if (pos->pieces[sq - 10] == EMPTY) {		// Se la casella di fronte al pedone (traversa precedente) e' vuota
														// la mossa e' possibile
				AddBlackPawnMove(pos,sq,sq - 10,list);	// Aggiunge mossa di non cattura alla lista

				if (RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY)	// Test su mossa di 2 caselle da posizione iniziale
					AddQuietMove(pos,MOVE(sq,sq - 20,					// Non e' certamente una mossa di promozione
						EMPTY, EMPTY, MFLAGPS),list);					// Aggiunge mossa 2 caselle da posizione iniziale
			}

			// Valutazione mossa con cattura non en-passant

			if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE)	// Se su diagonale destra c'e' un pezzo del Bianco 
				AddBlackPawnCapMove(pos,sq,sq - 9,pos->pieces[sq - 9],list);	// Aggiunge mossa di cattura di pedone alla lista

			if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE)// Se su diagonale sinistra c'e' un pezzo del Nero 
				AddBlackPawnCapMove(pos,sq,sq - 11,pos->pieces[sq - 11],list);	// Aggiunge mossa di cattura di pedone alla lista

			// Valutazione mossa con cattura en-passant

			if (pos->enPas != NO_SQ) {												// Propedeutico	
				if (sq - 9 == pos->enPas)											// Diagonale a destra
					AddEnPassantMove(pos,MOVE(sq,sq - 9,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant

				if (sq - 11 == pos->enPas)												// Diagonale a sinistra
					AddEnPassantMove(pos,MOVE(sq,sq - 11,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant
			}
		}

			// Gestione dell'arrocco del Nero

			if (pos->castlePerm & BKCA)						// Test se e' permesso l'arrocco del Nero sul lato di Re
				if (pos->pieces[F8] == EMPTY &&
					pos->pieces[G8] == EMPTY)				// In tal caso controlla che le caselle
															// tra Re e Torre in H8 siano vuote
					if (!SqAttacked(E8,WHITE) &&			// Controlla che il Re nero non sia sotto scacco
						!SqAttacked(F8,WHITE))				// Controlla che la casella di transito del Re non sia minacciata
						AddQuietMove(pos,MOVE(E8,G8,EMPTY,EMPTY,MFLAGCA),list);		// In tal caso l'arrocco e' una mossa ammessa
																					// e aggiunge la mossa di arrocco alla lista
															// NOTA: non si controlla che la casella G8 sia minacciata
															// poiche' comunque eseguita una qualunque mossa si
															// controllera' sempre che il Re non sia sotto scacco
															// prima di dichiara che la mossa eseguita e' legale
			if (pos->castlePerm & BQCA)						// Test se e' permesso l'arrocco del Nero sul lato di Donna
				if (pos->pieces[D8] == EMPTY &&
					pos->pieces[C8] == EMPTY &&
					pos->pieces[B8] == EMPTY)				// In tal caso controlla che le caselle
															// tra Re e Torre in A8 siano vuote
					if (!SqAttacked(E8,WHITE) &&			// Controlla che il Re nero non sia sotto scacco
						!SqAttacked(D8,WHITE))				// Controlla che la casella di transito del Re non sia minacciata
						AddQuietMove(pos,MOVE(E8,C8,EMPTY,EMPTY,MFLAGCA),list);		// In tal caso l'arrocco e' una mossa ammessa
																					// e aggiunge la mossa di arrocco alla lista
															// NOTA: non si controlla che la casella C8 sia minacciata
															// poiche' comunque eseguita una qualunque mossa si
															// controllera' sempre che il Re non sia sotto scacco
															// prima di dichiara che la mossa eseguita e' legale
	}

	// Gestione pezzi che minacciano a distanza (Alfiere,Torre,Regina)

	pceIndex = LoopSlideIndex[pos->side];						// Ricava indice di partenza nella lista dei pezzi

	while ( (pce = LoopSlidePce[pceIndex++]) != 0) {			// Loop fino alla fine della lista (che finisce con zero)

		assert(PieceValid(pce));								// Controlla validita' del codice del pezzo (EMPTY escluso)

		for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {		// Loop per tutti i pezzi di codice pce presenti
			
			sq = pos->pList[pce][pceNum];							// Ricava la casella del pezzo considerato	

			assert(SqOnBoard(sq));									// Controllo che la casella non sia invalida

			for (dir = 0; dir < NumDir[pce]; dir++) {				// Loop per tutte le direzioni di movimento del pezzo

				t_sq = sq;											// Init casella per esplorazione "raggio"

				while (!SQOFFBOARD( ( t_sq += PceDir[pce][dir] ) ))	// Procedi solo se la prossima casella e' valida
					if (pos->pieces[t_sq] == EMPTY)
						AddQuietMove(pos,MOVE(sq,t_sq,EMPTY,EMPTY,0),list);	// Se la casella target e' vuota e' una mossa
																			// non di cattura aggiunge la mossa alla lista
					else
					{
						if (PieceCol[pos->pieces[t_sq]] == (pos->side ^ BLACK))					// Con l'exclusive OR con BLACK ottiene il codice della parte avversa
							AddCaptureMove(pos,MOVE(sq,t_sq,pos->pieces[t_sq],EMPTY,0),list);	// Se nella casella target
																								// c'e' un pezzo della parte avversa
																								// e' una mossa di cattura
																								// e aggiunge la mossa alla lista
						break;											// Poi passa alla prossima direzione
					}
			}
		}
	}

	// Gestione pezzi che non minacciano a distanza (Cavallo,Re)

	pceIndex = LoopNonSlideIndex[pos->side];						// Ricava indice di partenza nella lista dei pezzi

	while ( (pce = LoopNonSlidePce[pceIndex++]) != 0) {				// Loop fino alla fine della lista (che finisce con zero)

		assert(PieceValid(pce));									// Controlla validita' del codice del pezzo (EMPTY escluso)

		for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {		// Loop per tutti i pezzi di codice pce presenti
			
			sq = pos->pList[pce][pceNum];							// Ricava la casella del pezzo considerato	

			assert(SqOnBoard(sq));									// Controllo che la casella non sia invalida

			for (dir = 0; dir < NumDir[pce]; dir++) {				// Loop per tutte le direzioni di movimento del pezzo

				t_sq = sq + PceDir[pce][dir];						// Ricava possibile caselle di arrivo

				if (!SQOFFBOARD(t_sq))								// Se la casella appartiene alla scacchiera reale
					if (pos->pieces[t_sq] == EMPTY)
						AddQuietMove(pos,MOVE(sq,t_sq,EMPTY,EMPTY,0),list);						// Se la casella target e' vuota e' una mossa
																								// non di cattura aggiunge la mossa alla lista
					else
						if (PieceCol[pos->pieces[t_sq]] == (pos->side ^ BLACK))					// Con l'exclusive OR con BLACK ottiene il codice della parte avversa
							AddCaptureMove(pos,MOVE(sq,t_sq,pos->pieces[t_sq],EMPTY,0),list);	// Se nella casella target
																								// c'e' un pezzo della parte
																								// avversa e' una mossa
																								// di cattura e aggiunge
																								// la mossa alla lista
			}			
		}
	}

	assert(MoveListOk(list,pos));									// Controllo validita' lista prodotta
}

//
// Funzione che genera la lista di tutte le mosse possibili di cattura in una posizione
//
// INPUT:	pos		pointer alla posizione
//			list	pointer alla lista da generare

void GenerateAllCaps(S_BOARD *pos,S_MOVELIST *list) {

	int pceNum;							// Indice corrente nella lista dei pezzi presenti di un certo tipo
	int sq;								// Casella interessata nella scacchiera estesa
	int pceIndex;						// Indice nella lista dei pezzi che minacciano a distanza e non
	int pce;							// Codice pezzo corrente nella lista
	int dir;							// Direzione corrente
	int t_sq;							// Appoggio per sq

	assert(CheckBoard(pos));			// Controlla che la posizione sia coerente

	list->count = 0;					// Dichiara vuota la lista da generare

	// Gestione del Pedone

	if (pos->side == WHITE) {				// Test se deve gestire una mossa del Bianco

		// Gestione del Pedone del Bianco
		
		for (pceNum = 0; pceNum < pos->pceNum[wP]; pceNum++) {		// Loop per tutti i pedoni bianchi presenti nella posizione

			sq = pos->pList[wP][pceNum];							// Ricava la casella di presenza del pedone bianco da valutare

			assert(SqOnBoard(sq));				// Controllo che la casella non sia invalida


			// Valutazione mossa con cattura non en-passant

			if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK)	// Se su diagonale sinistra c'e' un pezzo del Nero 
				AddWhitePawnCapMove(pos,sq,sq + 9,pos->pieces[sq + 9],list);	// Aggiunge mossa di cattura di pedone alla lista

			if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK)// Se su diagonale destra c'e' un pezzo del Nero 
				AddWhitePawnCapMove(pos,sq,sq + 11,pos->pieces[sq + 11],list);	// Aggiunge mossa di cattura di pedone alla lista

			// Valutazione mossa con cattura en-passant

			if (pos->enPas != NO_SQ) {												// Deve essere attiva la casella en-passant (altrimenti NO_SQ che vale 99 puo' eguagliare sq+11 se sq=88 (casella h7) !!! )
				if (sq + 9 == pos->enPas)											// Diagonale a sinistra
					AddEnPassantMove(pos,MOVE(sq,sq + 9,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant

				if (sq + 11 == pos->enPas)											// Diagonale a destra
					AddEnPassantMove(pos,MOVE(sq,sq + 11,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant
			}
		}
	}
	else {

		// Gestione del Pedone del Nero

		for (pceNum = 0; pceNum < pos->pceNum[bP]; pceNum++) {		// loop per tutti i pedoni neri presenti nella posizione

			sq = pos->pList[bP][pceNum];							// ricava la casella di presenza del pedone nero da valutare

			assert(SqOnBoard(sq));				// Controllo che la casella non sia invalida

				// Valutazione mossa con cattura non en-passant

				if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE)	// Se su diagonale destra c'e' un pezzo del Bianco 
					AddBlackPawnCapMove(pos,sq,sq - 9,pos->pieces[sq - 9],list);	// Aggiunge mossa di cattura di pedone alla lista

				if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE)// Se su diagonale sinistra c'e' un pezzo del Nero 
					AddBlackPawnCapMove(pos,sq,sq - 11,pos->pieces[sq - 11],list);	// Aggiunge mossa di cattura di pedone alla lista

				// Valutazione mossa con cattura en-passant

				if (pos->enPas != NO_SQ) {												// Propedeutico
					if (sq - 9 == pos->enPas)											// Diagonale a destra
						AddEnPassantMove(pos,MOVE(sq,sq - 9,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant

					if (sq - 11 == pos->enPas)												// Diagonale a sinistra
						AddEnPassantMove(pos,MOVE(sq,sq - 11,EMPTY,EMPTY,MFLAGEP),list);	// Aggiunge mossa di cattura en-passant
				}
		}
	}

	// Gestione pezzi che minacciano a distanza (Alfiere,Torre,Regina)

	pceIndex = LoopSlideIndex[pos->side];						// Ricava indice di partenza nella lista dei pezzi

	while ( (pce = LoopSlidePce[pceIndex++]) != 0) {			// Loop fino alla fine della lista (che finisce con zero)

		assert(PieceValid(pce));								// Controlla validita' del codice del pezzo (EMPTY escluso)

		for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {		// Loop per tutti i pezzi di codice pce presenti
			
			sq = pos->pList[pce][pceNum];							// Ricava la casella del pezzo considerato	

			assert(SqOnBoard(sq));									// Controllo che la casella non sia invalida

			for (dir = 0; dir < NumDir[pce]; dir++) {				// Loop per tutte le direzioni di movimento del pezzo

				t_sq = sq;											// Init casella per esplorazione "raggio"

				while (!SQOFFBOARD( ( t_sq += PceDir[pce][dir] ) ))								// Procedi solo se la prossima casella e' valida
					if (pos->pieces[t_sq] != EMPTY) {											// Se la casella di arrivo non e' vuota e' una mossa di cattura
						if (PieceCol[pos->pieces[t_sq]] == (pos->side ^ BLACK))					// Con l'exclusive OR con BLACK ottiene il codice della parte avversa
							AddCaptureMove(pos,MOVE(sq,t_sq,pos->pieces[t_sq],EMPTY,0),list);	// Se nella casella target
																								// c'e' un pezzo della parte avversa
																								// e' una mossa di cattura
																								// e aggiunge la mossa alla lista
						break;																	// Poi passa alla prossima direzione
					}
			}
		}
	}

	// Gestione pezzi che non minacciano a distanza (Cavallo,Re)

	pceIndex = LoopNonSlideIndex[pos->side];						// Ricava indice di partenza nella lista dei pezzi

	while ( (pce = LoopNonSlidePce[pceIndex++]) != 0) {				// Loop fino alla fine della lista (che finisce con zero)

		assert(PieceValid(pce));									// Controlla validita' del codice del pezzo (EMPTY escluso)

		for (pceNum = 0; pceNum < pos->pceNum[pce]; pceNum++) {		// Loop per tutti i pezzi di codice pce presenti
			
			sq = pos->pList[pce][pceNum];							// Ricava la casella del pezzo considerato	

			assert(SqOnBoard(sq));									// Controllo che la casella non sia invalida

			for (dir = 0; dir < NumDir[pce]; dir++) {				// Loop per tutte le direzioni di movimento del pezzo

				t_sq = sq + PceDir[pce][dir];						// Ricava possibile caselle di arrivo

				if (!SQOFFBOARD(t_sq))															// Se la casella appartiene alla scacchiera reale
					if (pos->pieces[t_sq] != EMPTY)												// Se la casella di arrivo non e' vuota e' una mossa di cattura
						if (PieceCol[pos->pieces[t_sq]] == (pos->side ^ BLACK))					// Con l'exclusive OR con BLACK ottiene il codice della parte avversa
							AddCaptureMove(pos,MOVE(sq,t_sq,pos->pieces[t_sq],EMPTY,0),list);	// Se nella casella target
																								// c'e' un pezzo della parte
																								// avversa e' una mossa
																								// di cattura e aggiunge
																								// la mossa alla lista
			}			
		}
	}

	assert(MoveListOk(list,pos));									// Controllo validita' lista prodotta
}