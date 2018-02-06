#include "defs.h"

//
// Questa matrice contiene le maschere da applicare all'informazione
// dei permessi per l'arrocco quando una mossa interessa la corrispondente
// casella nella scacchiera estesa.
// Le caselle che hanno 0xf come corrispondente NON altereranno i
// permessi dell'arrocco (mascherare con 0xf lascia inalterati i 4 bit dei permessi)
// Come si vede solo le caselle corrispondenti alle posizioni iniziali
// delle 4 Torri e dei 2 Re hanno valori diversi da 0xf
//
// Infatti sono le mosse di Torre e Re dalle loro caselle iniziali
// che alterano i permessi dell'arrocco.
//
// Se si muove la torre in A1 il Bianco non puo' piu' arroccare sul lato di Donna (maschera 0x0d = abbassa il bit 1)
// Se si muove la torre in H1 il Bianco non puo' piu' arroccare sul lato di Re (maschera 0x0e = abbassa il bit 0)
// Se si muove la torre in A8 il Nero non puo' piu' arroccare sul lato di Donna (maschera 0x07 = abbassa il bit 3)
// Se si muove la torre in H8 il Nero non puo' piu' arroccare sul lato di Re (maschera 0x0b = abbassa il bit 2)

// Se si muove il Re in E1 il Bianco non puo' piu' arroccare da entrambi i lati (maschera 0x0c = abbassa i bit 0 e 1)
// Se si muove il Re in E8 il Nero non puo' piu' arroccare da entrambi i lati (maschera 0x03 = abbassa i bit 2 e 3)

int CastlePerm[] = {

	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,

	0x0f,  0x0d,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0e,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x07,0x0f,0x0f,0x0f,0x03,0x0f,0x0f,0x0b,  0x0f,

	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f,
	0x0f,  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,  0x0f
};


//
// Funzione che cancella un pezzo da una casella
//
// INPUT:	sq		casella da svuotare
//			pos		pointer alla posizione

void ClearPiece(int sq, S_BOARD *pos) {

	int pce;						// Codice pezzo presente nella casella
	int col;						// Colore del pezzo presente nella casella
	int index;						// Indice esplorazione lista pList
	int t_pceNum;					// Indice dell'elemento trovato in lista pList

	assert(SqOnBoard(sq));			// Verifica validita' della casella da svuotare

	pce = pos->pieces[sq];			// Ricava codice pezzo in casella

	assert(PieceValid(pce));		// Controllo che il codice pezzo sia valido
									// (con EMPTY fallisce, per cui la casella NON puo' essere vuota)

	col = PieceCol[pce];			// Ricava colore in base al codice del pezzo

	HASH_PCE(pce,sq);				// La casella sq svuotandosi non contribuira' piu' all'hashkey 

	pos->pieces[sq] = EMPTY;		// Dichiara la casella sq vuota

	pos->material[col] -= PieceVal[pce];	// Toglie al punteggio del materiale della parte interessata
											// quello relativo al pezzo che era nella casella svuotata

	if (PieceBig[pce]) {					// Se il pezzo tolto dalla casella e' una figura

		pos->bigPce[col]--;					// decrementa il counter delle figure per la parte interessata

		if (PieceMaj[pce])					// Se il pezzo tolto dalla casella e' una figura maggiore
			pos->majPce[col]--;				// decrementa il counter delle figure maggiori per la parte interessata
		else
			pos->minPce[col]--;				// altrimenti decrementa il counter delle figure minori per la parte interessata
	}
	else {									// Altrimenti deve essere per forza un pedone : aggiorna le bitboard

		CLRBIT(pos->pawns[col],SQ64(sq));	// Abbassa il bit relativo alla casella sq nella bitboard della parte interessata
		CLRBIT(pos->pawns[BOTH],SQ64(sq));	// Abbassa il bit relativo alla casella sq nella bitboard relativa a entrambe le parti
	}

	// Aggiorna la lista pList

	t_pceNum = -1;							// Se per problemi l'elemento in lista non viene trovato rimarra'
											// in t_pceNum il valore di default che poi potra' essere controllato

	for (index = 0; index < pos->pceNum[pce]; index++)		// Loop per tutti i pezzi presenti del tipo uguale a quello eliminato
		if (pos->pList[pce][index] == sq) {					// Sicuramente deve essere presente in lista la casella del pezzo eliminato
			t_pceNum = index;								// In tal caso salva l'indice dell'elemento della lista
															// che sara' da eliminare
			break;											// esce dalla ricerca
		}

	assert(t_pceNum != -1);					// L'elemento in lista doveva esistere!

	pos->pceNum[pce]--;						// Decrementa il numero di casella in lista per il pezzo di codice pce

	// Sovrascrive il posto nella lista che era relativo al pezzo che e' stato tolto con l'ultimo elemento della lista
	// Ora la lista ha un elemento in meno e l'ultimo elemento di prima ha preso il posto di quello che era relativo al pezzo tolto
	// Il penultimo elemento ora e' l'ultimo della nuova lista che ha appunto un elemento in meno

	pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]];

	if (pce == wK) pos->KingSq[WHITE] = NO_SQ;		// Se e' un Re bianco rende invalida la locazione dedicata
	if (pce == bK) pos->KingSq[BLACK] = NO_SQ;		// Se e' un Re Nero rende invalida la locazione dedicata
};


//
// Funzione che aggiunge un pezzo in una casella vuota
//
// INPUT:	sq		casella vuota in cui aggiungere il pezzo
//			pos		pointer alla posizione
//			pce		codice pezzo da aggiungere

void AddPiece(int sq, S_BOARD *pos,int pce) {

	int col;						// Colore del pezzo presente nella casella

	assert(PieceValid(pce));		// Controllo che il codice pezzo sia valido
	assert(SqOnBoard(sq));			// Verifica validita' della casella da svuotare

	col = PieceCol[pce];			// Ricava colore in base al codice del pezzo

	HASH_PCE(pce,sq);				// Aggiorna l'hashkey con il pezzo pce nella casella sq

	pos->pieces[sq] = pce;			// Dichiara la presenza del pezzo pce nella casella sq

	pos->material[col] += PieceVal[pce];	// Aggiunge al punteggio del materiale della parte interessata
											// quello relativo al pezzo aggiunto

	if (PieceBig[pce]) {					// Se il pezzo aggiunto dalla casella e' una figura

		pos->bigPce[col]++;					// incrementa il counter delle figure per la parte interessata

		if (PieceMaj[pce])					// Se il pezzo aggiunto e' una figura maggiore
			pos->majPce[col]++;				// incrementa il counter delle figure maggiori per la parte interessata
		else
			pos->minPce[col]++;				// altrimenti incrementa il counter delle figure minori per la parte interessata
	}
	else {									// Altrimenti deve essere per forza un pedone : aggiorna le bitboard

		SETBIT(pos->pawns[col],SQ64(sq));	// Alza il bit relativo alla casella sq nella bitboard della parte interessata
		SETBIT(pos->pawns[BOTH],SQ64(sq));	// Alza il bit relativo alla casella sq nella bitboard relativa a entrambe le parti
	}

	pos->pList[pce][pos->pceNum[pce]++] = sq;	// Inserisce in coda alla pList del pezzo pce il numero della casella e
												// incrementa il numero dei pezzi in lista

	if (pce == wK) pos->KingSq[WHITE] = sq;		// Se e' un Re bianco salva la casella in cui e' posizionato
	if (pce == bK) pos->KingSq[BLACK] = sq;		// Se e' un Re Nero salva la casella in cui e' posizionato
};


//
// Funzione che sposta un pezzo da una casella a un'altra casella vuota
//
// INPUT:	from	casella da cui togliere il pezzo
//			to		casella in cui aggiungere il pezzo
//			pos		pointer alla posizione


void MovePiece(int from,int to, S_BOARD *pos) {

	int col;						// Colore del pezzo presente nella casella
	int pce;						// Codice pezzo da spostare
	int index;						// Indice in ricerca pList
	int t_pieceNum;					// Flag elemento trovato in pList

	assert(SqOnBoard(from));		// Verifica validita' della casella da cui togliere il pezzo
	assert(SqOnBoard(to));			// Verifica validita' della casella in cui aggiungere il pezzo

	pce = pos->pieces[from];		// Ricava il codice pezzo da spostare

	assert(PieceValid(pce));		// Controllo che il codice pezzo sia valido

	col = PieceCol[pce];			// Ricava colore in base al codice del pezzo

	HASH_PCE(pce,from);				// Aggiorna l'hashkey con il pezzo pce nella casella di partenza from

	pos->pieces[from] = EMPTY;		// Dichiara la casella from vuota

	HASH_PCE(pce,to);				// Aggiorna l'hashkey con il pezzo pce nella casella di arrivo to

	pos->pieces[to] = pce;			// Dichiara nella casella to il pezzo di codice pce

	if (!PieceBig[pce]) {					// Se il pezzo aggiunto dalla casella non e' una figura (e quindi e' un pedone)

		CLRBIT(pos->pawns[col],SQ64(from));		// Azzera il bit relativo alla casella from nella bitboard della parte interessata
		CLRBIT(pos->pawns[BOTH],SQ64(from));	// Azzera il bit relativo alla casella from nella bitboard relativa a entrambe le parti
		SETBIT(pos->pawns[col],SQ64(to));		// Alza il bit relativo alla casella to nella bitboard della parte interessata
		SETBIT(pos->pawns[BOTH],SQ64(to));		// Alza il bit relativo alla casella to nella bitboard relativa a entrambe le parti
	}

	// Aggiornamento pList

	t_pieceNum = FALSE;										// Default flag elemento trovato in pList

	for (index = 0; index < pos->pceNum[pce]; index++)		// Cerca in pList la casella from per il pezzo di codice pce
		if (pos->pList[pce][index] == from) {
			pos->pList[pce][index] = to;					// Aggiorna la casella in cui il pezzo pce si trova
			t_pieceNum = TRUE;								// Segnalazione elemento trovato
			break;											// esce dalla ricerca
		}

	assert(t_pieceNum);										// Controllo poiche' l'elemento in pList DEVE esistere!
};


//
// Funzione che aggiorna la posizione in base a una mossa
//
// INPUT:	move		mossa
// OUTPUT:	FALSE		Alla fine della mossa il Re della parte che ha mossa e' sotto scacco
//						La mossa proposta e' illegale e viene non viene eseguita
//			TRUE		La mossa e' okay

int MakeMoveV(int move) {

	int from;							// Casella di partenza del pezzo che si muovera'
	int to;								// Casella di arrivo del pezzo che si muovera'
	int side;							// Colore della parte che muove
	int captured;						// Codice pezzo eventualmente catturato
	int prPce;							// Codice pezzo promosso
	
	assert(CheckBoard(pos));			// Controllo coerenza della posizione di partenza

	from = FROMSQ(move);				// Ricava casella di partenza
	to = TOSQ(move);					// Ricava casella di arrivo
	side = pos->side;					// Ricava parte che ha la mossa

	assert(SqOnBoard(from));					// Controllo validita' casella di partenza
	assert(SqOnBoard(to));						// Controllo validita' casella di arrivo
	assert(SideValid(side));					// Controllo validita' parte che deve muovere
	assert(PieceValid(pos->pieces[from]));		// Controllo che il codice pezzo nella casella from sia corretto
												// NOTA: EMPTY e' considerato un codice non valido: la casella non puo' essere vuota
	
	pos->history[pos->hisPly].posKey = pos->posKey;		// Prima di alterare la posizione e quindi anche l'hashkey
														// salva l'hashkey della posizione di partenza nella history list
														// Successivamente salvera' il resto

	if (move & MFLAGEP)							// Test se mossa di presa en-passant
		if (side == WHITE)
			ClearPiece(to - 10,pos);			// Se la presa en-passant e' del Bianco rimuove il pedone nero che si trova
												// nella traversa inferiore
		else
			ClearPiece(to + 10,pos);			// Se la presa en-passant e' del Nero rimuove il pedone bianco che si trova
												// nella traversa superiore

	if (move & MFLAGCA)							// Test se mossa di arrocco
		switch (to) {							// Discrimina il tipo di arrocco dalla casella di arrivo del Re

		case G1:	MovePiece(H1,F1,pos);		// Arrocco Bianco lato di Re : muove la Torre bianca da H1 a F1
					break;
		case C1:	MovePiece(A1,D1,pos);		// Arrocco Bianco lato di Donna: muove la Torre bianca da A1 a D1
					break;
		case G8:	MovePiece(H8,F8,pos);		// Arrocco Nero lato di Re: muove la Torre nera da H8 a F8
					break;
		case C8:	MovePiece(A8,D8,pos);		// Arrocco Nero lato di Donna: muove la Torre nera da A8 a D8
					break;

		default:	assert(FALSE);				// Errore!
					break;
	}

	if (pos->enPas != NO_SQ)
		HASH_EP;				// Annulla contributo all'hashkey della casella en-passant presente nella posizione di partenza
	
	HASH_CA;					// Annulla contributo all'hashkey dei permessi di arrocco presenti nella posizione di partenza

	// Completa le informazioni nella history list con le informazioni della posizione di partenza

	pos->history[pos->hisPly].move = move;						// Mossa
	pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;		// Counter per regola delle 50 mosse
	pos->history[pos->hisPly].enPas = pos->enPas;				// Casella en-passant
	pos->history[pos->hisPly++].castlePerm = pos->castlePerm;	// Permessi di arrocco e aggiorna indice alla history

	// Adesso comincia ad aggiornare la posizione pos

	pos->castlePerm &= CastlePerm[from];		// Annulla eventualmente i permessi di arrocco in base alla tabella CastlePerm
												// relativamente alla casella di partenza. E' significativo quando il pezzo
												// mosso e' il Re o una Torre
	pos->castlePerm &= CastlePerm[to];			// Annulla eventualmente i permessi di arrocco in base alla tabella CastlePerm
												// relativamente alla casella di arrivo. E' significativo se la mossa e' di
												// cattura di una Torre che si trova ancora nella casa iniziale

	pos->enPas = NO_SQ;							// Disattiva per default la casella en-passant

	pos->fiftyMove++;							// Ora aggiorna il counter per la regola delle 50 mosse

	captured = CAPTURED(move);					// Preleva codice pezzo catturato nella mossa

	assert(PieceValidEmpty(captured));			// Controllo codice pezzo catturato (puo' valere EMPTY)

	if (captured != EMPTY) {					// Test se effettivamente la mossa era di presa
	
		ClearPiece(to,pos);						// Elimina il pezzo presente nella casella di arrivo
		pos->fiftyMove = 0;						// La presa fa' azzerare il counter per la regola delle 50 mosse
	}

	pos->ply++;									// Aggiorna depth della ricerca

	if (PiecePawn[pos->pieces[from]]) {			// Se il pezzo che si muove e' un pedone
	
		pos->fiftyMove = 0;						// Una mossa di pedone fa' azzerare il counter per la regola delle 50 mosse
	
		if (move & MFLAGPS)	{					// Se e' una mossa di 2 caselle da posizione di partenza
			if (side == WHITE) {				// Se e' una mossa del Bianco
				pos->enPas = from + 10;			// La casella enpassant e' quella nella reaversa superiore della casella di partenza
				assert(RanksBrd[pos->enPas] == RANK_3);		// Ora la traversa della casella enpassant deve essere la 3
			}
			else {								// Se invece e' una mossa del Nero
				pos->enPas = from - 10;			// La casella enpassant e' quella nella reaversa inferiore della casella di partenza
				assert(RanksBrd[pos->enPas] == RANK_6);		// Ora la traversa della casella enpassant deve essere la 6
			}

			HASH_EP;							// Aggiorna il contributo all'hashkey della casella en-passant
		}
	}

	HASH_CA;								// Aggiorna il contributo all'hashkey dei permessi aggiornati

	MovePiece(from,to,pos);					// Infine spostiamo il pezzo della mossa nella posizione pos

	prPce = PROMOTED(move);					// Ricava il codice del pezzo promosso

	assert(PieceValidEmpty(prPce));			// Controlla la validita' del codice del pezzo promosso (puo' valere EMPTY)

	if (prPce != EMPTY) {					// Se c'e' effettivamente un pezzo promosso
	
		assert(!PiecePawn[prPce]);				// Non puo' comunque essere un pedone!

		ClearPiece(to,pos);						// Toglie il pedone promosso

		AddPiece(to,pos,prPce);					// e lo sostituisce con il pezzo promosso
	}

	if (PieceKing[pos->pieces[to]])				// Se il pezzo mosso e' il Re
		pos->KingSq[side] = to;					// Aggiorna l'info dedicata

	pos->side ^= BLACK;							// Ora tocca all'avversario muovere

	HASH_SIDE;									// Aggiorna hashkey relativamente al contributo della parte che ha la mossa

	if (SqAttacked(pos->KingSq[side],pos->side)) {			// Se il Re della parte che ha appena mosso e' sotto attacco dalla
															// parte che ora deve muovere la mossa e' illegale e va' annullata
	
		TakeMove(pos);										// Annulla la mossa

		return FALSE;										// Segnala mossa illegale

	}

	assert(CheckBoard(pos));			// Controllo coerenza della posizione raggiunta

	return TRUE;
}


//
// Funzione per tornare indietro di una mossa
//
// INPUT:	pos		pointer alla posizione
//

void TakeMove(S_BOARD *pos) {

	int move;							// Mossa da invertire
	int from;							// Casella di partenza della mossa da invertire
	int to;								// Casella di arrivo della mossa da invertire
	int captured;						// Codice pezzo catturato
	int promoted;						// Codice pezzo promosso

	assert(CheckBoard(pos));			// Controllo coerenza della posizione

	pos->hisPly--;						// Decrementa indice in lista history in modo che punti alle informazioni della
										// posizione da ripristinare
	pos->ply--;							// Decrementa depth della ricerca

	move = pos->history[pos->hisPly].move;		// Recupera mossa da invertire
	from = FROMSQ(move);						// Recupera casella di partenza da mossa da invertire
	to = TOSQ(move);							// Recuper casella di arrivo da mossa da invertire

	assert(SqOnBoard(from));			// Controllo validita' casella di partenza	
	assert(SqOnBoard(to));				// Controllo validita' casella di arrivo

	if (pos->enPas != NO_SQ)
		HASH_EP;						// Annulla contributo hashkey della casella enpassant attuale

	HASH_CA;							// Annulla contributo hashkey dei permessi di arrocco attuali

	pos->castlePerm = pos->history[pos->hisPly].castlePerm;		// Recupero i permessi d'arrocco della posizione da ripristinare
	pos->enPas = pos->history[pos->hisPly].enPas;				// Recupero casella en-passnt della posizione da ripristinare
	pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;		// Recupero counter per la regola delle 50 mosse
																// della posizione da ripristinare
	if (pos->enPas != NO_SQ)
		HASH_EP;						// Aggiorna contributo alla hashkey della casella enpassant attiva

	HASH_CA;							// Aggiorna contributo alla hashkey dei permessi dell'arrocco

	pos->side ^= BLACK;					// Ora la mossa va' alla parte avversa

	HASH_SIDE;							// Aggiorna contributo di chi ha la mossa all'hashkey della posizione

	if (move & MFLAGEP)					// Se la mossa da invertire era una presa en-passant
		if (pos->side == WHITE)			
			AddPiece(to - 10,pos,bP);	// Se era una presa en-passant del Bianco bisognera' ripristinare il pedone Nero
										// catturato che era nella traversa inferiore rispetto la casella di arrivo della presa
		else
			AddPiece(to + 10,pos,wP);	// Se era una presa en-passant del Nero bisognera' ripristinare il pedone Bianco
										// catturato che era nella traversa superiore rispetto la casella di arrivo della presa

	if (move & MFLAGCA)					// Se era una mossa di arrocco
		switch (to) {					// in base alla casella di arrivo del Re ricava il tipo di arrocco e sposta 
										// la torre nella casella di partenza
		case G1:	MovePiece(F1,H1,pos);		// Arrocco del Bianco sul lato di Re
					break;
		case C1:	MovePiece(D1,A1,pos);		// Arrocco del Bianco sul lao di Donna
					break;
		case G8:	MovePiece(F8,H8,pos);		// Arrocco del Nero sul lato di Re
					break;
		case C8:	MovePiece(D8,A8,pos);		// Arrocco del Nero sul lato di Donna
					break;

		default:	assert(FALSE);				// Errore!
					break;
	}

	MovePiece(to,from,pos);				// Sposta il pezzo della mossa dalla casella di arrivo di nuovo alla casella di partenza

	if (PieceKing[pos->pieces[from]])	// Se il pezzo ripristinato e' un Re
		pos->KingSq[pos->side] = from;	// Aggiorna l'informazione dedicata

	captured = CAPTURED(move);

	assert(PieceValidEmpty(captured));	// Controllo codice pezzo catturato (EMPTY e' valido)

	if (captured != EMPTY)				// Se effettivamente la mossa prevedeva di catturare un pezzo
		AddPiece(to,pos,captured);		// Ripristina il pezzo catturato nella casella di arrivo della mossa da invertire

	promoted = PROMOTED(move);			// Ricava codice pezzo promosso

	assert(PieceValidEmpty(promoted));	// Controllo codice pezzo promosso (EMPTY e' valido)

	if (promoted != EMPTY) {
	
		assert(!PiecePawn[promoted]);									// Il pezzo promosso non puo' essere un pedone
		ClearPiece(from,pos);											// Elimina il pezzo promosso
		AddPiece(from,pos,( PieceCol[promoted] == WHITE ? wP : bP ) );	// E al suo posto ripristina il pedone del colore corretto
	}

	assert(CheckBoard(pos));		// Controllo coerenza posizione ripristinata
}


//
// Funzione che attua la cosiddetta mossa nulla (Null Move)
//

void MakeNullMove(void) {

	assert(CheckBoard(pos));			// Controllo coerenza della posizione di partenza
	assert(!SqAttacked(pos->KingSq[pos->side],pos->side ^ BLACK));		// Per potere eseguire la mossa nulla non si deve essere sotto scacco

	// Salva le informazioni nella history list con le informazioni della posizione di partenza

	pos->history[pos->hisPly].move = NOMOVE;					// Mossa nulla
	pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;		// Counter per regola delle 50 mosse
	pos->history[pos->hisPly].enPas = pos->enPas;				// Casella en-passant
	pos->history[pos->hisPly].castlePerm = pos->castlePerm;		// Permessi di arrocco e aggiorna indice alla history
	pos->history[pos->hisPly++].posKey = pos->posKey;			// Hashkey

	// Adesso comincia ad aggiornare la posizione pos
	
	if (pos->enPas != NO_SQ)
		HASH_EP;				// Annulla contributo all'hashkey della casella en-passant presente nella posizione di partenza

	pos->enPas = NO_SQ;			// Disattiva per default la casella en-passant

	pos->side ^= BLACK;			// Ora tocca all'avversario muovere

	HASH_SIDE;					// Aggiorna hashkey relativamente al contributo della parte che ha la mossa
		
	pos->ply++;					// Aggiorna depth della ricerca

	assert(CheckBoard(pos));	// Controllo coerenza della posizione raggiunta
}


//
// Funzione per tornare indietro di una mossa nulla (Null Move
//

void TakeNullMove(void) {

	assert(CheckBoard(pos));			// Controllo coerenza della posizione

	pos->hisPly--;						// Decrementa indice in lista history in modo che punti alle informazioni della
										// posizione da ripristinare
	pos->ply--;							// Decrementa depth della ricerca

	if (pos->enPas != NO_SQ)
		HASH_EP;						// Annulla contributo hashkey della casella enpassant attuale

	pos->castlePerm = pos->history[pos->hisPly].castlePerm;		// Recupero i permessi d'arrocco della posizione da ripristinare
	pos->enPas = pos->history[pos->hisPly].enPas;				// Recupero casella en-passnt della posizione da ripristinare
	pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;		// Recupero counter per la regola delle 50 mosse
																// della posizione da ripristinare
	if (pos->enPas != NO_SQ)
		HASH_EP;						// Aggiorna contributo alla hashkey della casella enpassant attiva

	pos->side ^= BLACK;					// Ora la mossa va' alla parte avversa

	HASH_SIDE;							// Aggiorna contributo di chi ha la mossa all'hashkey della posizione

	assert(CheckBoard(pos));		// Controllo coerenza posizione ripristinata
}
