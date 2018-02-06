#include "defs.h"

//
// Funzione che controllo la coerenza delle informazioni presenti in una posizione
//
//	INPUT:	pos		pointer alla posizione
//
//	OUTPUT:	TRUE	OKAY

int CheckBoard(S_BOARD *pos) {

	int t_piece;		// Codice tipo pezzo
	int t_pce_num;		// Counter pezzi del tipo selezionato
	int colour;			// Colore del pezzo interessato
	int sq;				// Casella in scacchiera reale

	int t_pceNum[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		// Copia da ricalcolare dell'analoga informazione presente nella posizione
	int t_bigPce[] = { 0, 0 };										// Copia da ricalcolare dell'analoga informazione presente nella posizione
	int t_majPce[] = { 0, 0 };										// Copia da ricalcolare dell'analoga informazione presente nella posizione
	int t_minPce[] = { 0, 0 };										// Copia da ricalcolare dell'analoga informazione presente nella posizione
	int t_material[] = { 0, 0 };									// Copia da ricalcolare dell'analoga informazione presente nella posizione

	U64 t_pawns[3];													// Copia dell'analoga informazione presente nella posizione

	// Controllo coerenza contenuto della pList con i pezzi dichiarati nella posizione

	for (t_piece = wP; t_piece <= bK; t_piece++)								// Loop su tutti i tipi di pezzo
		for (t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; t_pce_num++)		// Loop su tutti i pezzi del tipo selezionato
			assert(pos->pieces[pos->pList[t_piece][t_pce_num]] == t_piece);		// Controllo tra contenuto pList e scacchiera estesa

	// Controlli coerenza counters per figure,fugre maggiori e figure minori

		// Prima ricalcolo i valori locali

	for (sq = 0; sq < 64; sq++) {									// Loop su 64 caselle
		
		t_piece = pos->pieces[SQ120(sq)];							// Ricava codice pezzo presente nella casella corrente

		t_pceNum[t_piece]++;										// Aggiorna counter locale per il numero di pezzi del tipo

		colour = PieceCol[t_piece];									// Ricava il colore

		if (PieceBig[t_piece] == TRUE) t_bigPce[colour]++;			// Aggiorna counter locale per il numero di figure
		if (PieceMaj[t_piece] == TRUE) t_majPce[colour]++;			// Aggiorna counter locale per il numero di figure maggiori
		if (PieceMin[t_piece] == TRUE) t_minPce[colour]++;			// Aggiorna counter locale per il numero di figure minori

		t_material[colour] += PieceVal[t_piece];					// Aggiorna lo score del colore interessato relativamente al pezzo selezionato
	}

		// Poi controllo la coerenza con i valori memorizzati nella posizione

	for (t_piece = wP; t_piece <= bK; t_piece++)					// Loop su tutti i tipi di pezzo
		assert(t_pceNum[t_piece] == pos->pceNum[t_piece]);			// Controllo su counters per ogni tipo di pezzo

		assert(t_bigPce[WHITE] == pos->bigPce[WHITE]);				// Controllo su numero figure bianche
		assert(t_majPce[WHITE] == pos->majPce[WHITE]);				// Controllo su numero figure bianche
		assert(t_minPce[WHITE] == pos->minPce[WHITE]);				// Controllo su numero figure bianche

		assert(t_bigPce[BLACK] == pos->bigPce[BLACK]);				// Controllo su numero figure nere
		assert(t_majPce[BLACK] == pos->majPce[BLACK]);				// Controllo su numero figure nere
		assert(t_minPce[BLACK] == pos->minPce[BLACK]);				// Controllo su numero figure nere

		assert(t_material[WHITE] == pos->material[WHITE]);			// Controllo su score materiale per il bianco
		assert(t_material[BLACK] == pos->material[BLACK]);			// Controllo su score materiale per il nero


	// Controlli di coerenza su bitboards

	t_pawns[WHITE] = pos->pawns[WHITE];
	t_pawns[BLACK] = pos->pawns[BLACK];
	t_pawns[BOTH]  = pos->pawns[BOTH];								// Prima copia le bitboard in variabili locali (questo perche' verranno alterate nel test)

		// Controllo sul numero dei pedoni

	assert(CNT(t_pawns[WHITE]) == pos->pceNum[wP]);						// Per il bianco
	assert(CNT(t_pawns[BLACK]) == pos->pceNum[bP]);						// Per il nero
	assert(CNT(t_pawns[BOTH]) == pos->pceNum[wP] + pos->pceNum[bP]);	// Per entrambi

		// Controllo sulle posizioni dei pedoni

	while (t_pawns[WHITE]) {
		sq = SQ120(POP(&t_pawns[WHITE]));
		assert(pos->pieces[sq] == wP);								// Per il bianco
	}
	while (t_pawns[BLACK]) {
		sq = SQ120(POP(&t_pawns[BLACK]));
		assert(pos->pieces[sq] == bP);								// Per il nero
	}
	while (t_pawns[BOTH]) {
		sq = SQ120(POP(&t_pawns[BOTH]));
		assert(pos->pieces[sq] == wP || pos->pieces[sq] == bP);		// Per entrambi
	}


	// Controllo sull'informazione della parte che deve muovere

	assert(pos->side == WHITE || pos->side == BLACK);

	// Controllo sull'hashkey

	assert(GeneratePosKey(pos) == pos->posKey);

	// Controllo sull'informazione della casella en-passant

	assert(pos->enPas == NO_SQ ||
		(pos->side == WHITE && RanksBrd[pos->enPas] == RANK_6) ||
		(pos->side == BLACK && RanksBrd[pos->enPas] == RANK_3));	// La casella en-passant o non e' valida o deve essere sempre sulle traverse 6 o 3 a seconda del colore che ha la mossa

	// Controllo sull'informazione della posizione dei Re

	assert(pos->pieces[pos->KingSq[WHITE]] == wK);
	assert(pos->pieces[pos->KingSq[BLACK]] == bK);

	return TRUE;
}

//
// Funzione che completa le informazioni di una posizione
//
// INPUT:	pos		pointer alla posizione
//

void UpdateListsMaterial(S_BOARD *pos) {

	int sq;			// Casella in scaxchiera estesa
	int piece;		// Codice del pezzo presente nella casella attuale
	int colour;		// Colore del pezzo attuale

	for (sq = 0; sq < BRD_SQ_NUM; sq++) {							// Loop su tutte le casella della scacchiera estesa

		piece = pos->pieces[sq];									// Preleva codice pezzo nella casella

		if (piece != OFFBOARD && piece != EMPTY) {					// Se la casella e' valida e non vuota

			colour = PieceCol[piece];								// Preleva il colore del pezzo

			if (PieceBig[piece] == TRUE) pos->bigPce[colour]++;		// Se e' una figura incrementa il counter delle figure per la parte di appartenenza
			if (PieceMin[piece] == TRUE) pos->minPce[colour]++;		// Se e' una figura maggiore incrementa il counter delle figure maggiori per la parte di appartenenza
			if (PieceMaj[piece] == TRUE) pos->majPce[colour]++;		// Se e' una figura minore incrementa il counter delle figure minori per la parte di appartenenza

			pos->material[colour] += PieceVal[piece];				// Aggiorna lo score relativo al materiale per la parte interessata

			pos->pList[piece][pos->pceNum[piece]++] = sq;			// Inserisce la casella nella lista relativa al codice pezzo e aggiorna il counter dei pezzi di quel codice

			if (piece == wK) pos->KingSq[WHITE] = sq;				// Se e' un Re bianco salva la casella in cui e' posizionato
			if (piece == bK) pos->KingSq[BLACK] = sq;				// Se e' un Re Nero salva la casella in cui e' posizionato
		
			if (piece==wP) {										// Se e' un pedone bianco
				SETBIT(pos->pawns[WHITE],SQ64(sq));					// Setta il bit corrspondente nella bitboard del bianco
				SETBIT(pos->pawns[BOTH],SQ64(sq));					// Setta il bit corrspondente nella bitboard per entrambi
			}

			if (piece==bP) {
				SETBIT(pos->pawns[BLACK],SQ64(sq));					// Setta il bit corrspondente nella bitboard del nero
				SETBIT(pos->pawns[BOTH],SQ64(sq));					// Setta il bit corrspondente nella bitboard per entrambi
			}
		}
	}
}

//
// Funzione che legge una posizione in formato FEN
//
// INPUT:	fen		pointer alla stringa fen
//
// OUTPUT:	1		OKAY

int ParseFen(char *fen) {

	int sq64;				// casella in bitboard
	char ch;				// carattere corrente nella stringa fen
	int index;				// indice corrente nella stringa fen
	int count;				// counter caselle vuote
	int piece;				// codice pezzo decodificato
	int i;					// indice loop
	int file;				// Colonna
	int rank;				// Traversa
	
	assert(fen!=NULL);
	assert(pos!=NULL);		// un minimo controllo sulla validita' dei pointer in input

	ResetBoard(pos);		// Per prima cosa settiamo una posizione vuota

	// Decodifica primo campo (disposizione pezzi)

	sq64 = SQ64(A8);		// Inizia dalla ottava riga prima colonna

	index=0;				// Inizia dal primo carattere della stringa fen

	while ( (sq64 >= 0) && ( (ch = fen[index++]) != '\0' && ch != ' ') )	{	// prosegue fino alla fine della prima riga oppure se incontra un carattere di fine stringa o un blank
		
		count = 0;																// No caselle vuote
		piece = EMPTY;															// Reset codice pezzo

		if (ch >= '1' && ch <= '8')												// test se numero indicante caselle vuote consecutive
			count = ch - '0';													// count = numero caselle consecutive vuote
		else
		{
			switch (ch)															// Decodifica se e' un carettere indicante un tipo di pezzo
			{
			case	'p':	piece = bP;											// Pedone nero
							break;
			case	'n':	piece = bN;											// Cavallo nero
							break;
			case	'b':	piece = bB;											// Alfiere nero
							break;
			case	'r':	piece = bR;											// Torre nera
							break;
			case	'q':	piece = bQ;											// Regina nera
							break;
			case	'k':	piece = bK;											// Re nero
							break;
			case	'P':	piece = wP;											// Pedone bianco
							break;
			case	'N':	piece = wN;											// Cavallo bianco
							break;
			case	'B':	piece = wB;											// Alfiere bianco
							break;
			case	'R':	piece = wR;											// Torre bianca
							break;
			case	'Q':	piece = wQ;											// Regina bianca
							break;
			case	'K':	piece = wK;											// Re bianco
							break;

			case	'/':	sq64 -= 16;											// Se carattere di fine riga va sulla prima colonna della riga precedente
							break;
			default:															// Errore per carattere non decodificato
					DoLog("FEN error \n");
					return -1;
			}
		}

		if (piece != EMPTY)														// Test se decodificato un pezzo
			pos->pieces[SQ120(sq64++)] = piece;									// Carica il codice pezzo nella scacchiera estesa e passa alla colonna successiva
		else
			while (count-- > 0)													// Carica eventuali caselle vuote consecutive (se decodificato '/' non fa niente poiche' in tal caso e' count=0)
				pos->pieces[SQ120(sq64++)] = EMPTY;								// Carica codice di casella vuota e passa alla colonna successiva
	}

	// Ora index punta al carattere che specifica quale parte ha la mossa

	// Decodifica secondo campo (side to move)

	assert(fen[index] == 'w' || fen[index] == 'b');								// Verifica sia un carattere valido

	pos->side = (fen[index] == 'w') ? WHITE : BLACK;							// Carica la parte che ha la mossa

	index += 2;																	// Passa al campo dei permessi dell'arrocco

	// Decodifica terzo campo (permessi dell'arrocco)

	i = 0;
	
	while ((ch = fen[index++]) != ' ' && i++ < 4)								// Il parse termina dopo un blank o comunque dopo 4 caratteri
		switch (ch) {															// NOTA: nel caso ci sia il carettere '-' non decodifica niente.
			case 'K':		pos->castlePerm |= WKCA;							// Alza il bit per l'arrocco bianco lato di re
							break;
			case 'Q':		pos->castlePerm |= WQCA;							// Alza il bit per l'arrocco bianco lato di donna
							break;
			case 'k':		pos->castlePerm |= BKCA;							// Alza il bit per l'arrocco nero lato di re
							break;
			case 'q':		pos->castlePerm |= BQCA;							// Alza il bit per l'arrocco nero lato di donna
							break;
		}

	assert(pos->castlePerm >=0 && pos->castlePerm <= 15);						// Controllo che il codici dei permessi dell'arrocco cosi' generato sia un codice valido

	// Ora index punta alla casella di en-passant

	// Decodifica del quarto campo

	if (fen[index] != '-') {														// Solo se casella en-passant attiva
	
		file = fen[index] - 'a';													// Ricava la colonna (da 0 a 7)
		rank = fen[index+1] - '1';													// Ricava la traversa (da 0 a 7)

		assert(file >= FILE_A && file <= FILE_H);
		assert(rank >= RANK_1 && rank <= RANK_8);									// Controllo che i valori decodificati siano validi

		pos->enPas = FR2SQ(file,rank);												// Carica nr. casella corrispondente in scacchiera estesa
	}

	//
	// Ora calcola l'hashkey della posizione
	//

	pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);														// Completa le informazioni

	return TRUE;
}


//
// Funzione per svuotare una posizione (scacchiera vuota)
//
// INPUT:	pos		pointer alla posizione da alterare
//

void ResetBoard(S_BOARD *pos) {

	int index;

	for (index = 0; index < BRD_SQ_NUM; index++)		// Dichiara tutte le caselle della scacchiera estesa come non valide
		pos->pieces[index] = OFFBOARD;

	for (index = 0; index < 64; index++)
		pos->pieces[SQ120(index)] = EMPTY;				// Le caselle effettivamente esistenti sono dichiarate ora vuote (ma valide!)

	for (index = 0; index < 2; index++) {				// Azzera i vari contatori dei pezzi presenti  e lo score del materiale
		pos->bigPce[index] = 0;
		pos->majPce[index] = 0;
		pos->minPce[index] = 0;
		pos->material[index] = 0;
	}

	for (index = 0; index < 3; index++)					// Azzera le bitboard dei pedoni
		pos->pawns[index] = 0ULL;

	for (index = 0; index < 13; index++)
		pos->pceNum[index] = 0;							// Azzera il counter dei pezzi di ogni tipo

	pos->KingSq[WHITE] = NO_SQ;							// Il Re bianco non e' presente
	pos->KingSq[BLACK] = NO_SQ;							// Il Re nero non e' presente

	pos->side = BOTH;									// Per non propendere ne' da una parte ne' dall'altra

	pos->enPas = NO_SQ;									// Nessuna casella en-passant attiva

	pos->fiftyMove = 0;									// Azzera counter della regola delle 50 mosse

	pos->ply = 0;										// Azzera il counter delle mezze mosse in fase di ricerca 
	pos->hisPly = 0;									// Azzera il counter delle mezze mosse della partita

	pos->castlePerm = 0;								// Azzera i permessi di arrocco

	pos->posKey = 0ULL;									// Azzera la hashkey della posizione

	// Il campo pList viene volutamente tralasciato : sara' impostato in un'altra funzione ( UpdateListMaterial() )
}

//
// Funzione che stampa la posizione in input
//
// INPUT:	pos		pointer alla posizione da stampare

void PrintBoard(const S_BOARD *pos) {

	int file;		// Nr. colonna
	int rank;		// Nr. traversa

	DoLog("\nGame Board:\n\n");

	for (rank = RANK_8; rank >= RANK_1; rank--) {					// Si parte dalla traversa 8 per arrivare alla fine alla traversa 1
		DoLog("%d  \r", rank + 1);									// Stampa numero traversa a partire da 1
		
		for (file = FILE_A; file <= FILE_H; file++)					// Dalla colonna A alla colonna H
			DoLog("%3c\r",PceChar[pos->pieces[FR2SQ(file,rank)]]);	// Stampa il carattere assegnandoli 3 digit indicizzandosi col codice del pezzo nel vattore dei simboli

		DoLog("\n");												// Alla fine della riga va' a capo
	}
	
	DoLog("\n   ");

	for (file = FILE_A; file <= FILE_H; file++)
		DoLog("%3c\r",'a' + file);									// Alla fine stampa una riga con i nomi delle colonne ('a','b',...,'h')

	DoLog("\n\n");

	DoLog("side:%c\n",SideChar[pos->side]);						// Stampa la parte che deve muovere ('w' o 'b')

	DoLog("enpas:%d\n",pos->enPas);								// Stampa la casella en-passant come decimale (che rozzo!)
	
	DoLog("castle:%c%c%c%c\n",pos->castlePerm & WKCA ? 'K' : '-',
							   pos->castlePerm & WQCA ? 'Q' : '-',
							   pos->castlePerm & BKCA ? 'k' : '-',
							   pos->castlePerm & BQCA ? 'q' : '-');	// Stampa 4 caratteri per i permessi dell'arrocco ('-' se il corrsipondente permesso non c'e')	

	DoLog("Poskey:%llX\n",pos->posKey);							// Stampa in esadecimale l'haskey della posizione (llX = long long hexadecimal)
}

//
// Funzione che stampa la posizione in input
//
// INPUT:	pos		pointer alla posizione da stampare

void PrintBoardUci() {

	int file;		// Nr. colonna
	int rank;		// Nr. traversa

	printf("\nGame Board:\n\n");

	for (rank = RANK_8; rank >= RANK_1; rank--) {					// Si parte dalla traversa 8 per arrivare alla fine alla traversa 1
		printf("%d  ", rank + 1);									// Stampa numero traversa a partire da 1
		
		for (file = FILE_A; file <= FILE_H; file++)					// Dalla colonna A alla colonna H
			printf("%3c",PceChar[pos->pieces[FR2SQ(file,rank)]]);	// Stampa il carattere assegnandoli 3 digit indicizzandosi col codice del pezzo nel vattore dei simboli

		printf("\n");												// Alla fine della riga va' a capo
	}
	
	printf("\n   ");

	for (file = FILE_A; file <= FILE_H; file++)
		printf("%3c",'a' + file);								// Alla fine stampa una riga con i nomi delle colonne ('a','b',...,'h')

	printf("\n\n");

	printf("side:%c\n",SideChar[pos->side]);					// Stampa la parte che deve muovere ('w' o 'b')

	printf("enpas:%d\n",pos->enPas);							// Stampa la casella en-passant come decimale (che rozzo!)
	
	printf("castle:%c%c%c%c\n",pos->castlePerm & WKCA ? 'K' : '-',
							   pos->castlePerm & WQCA ? 'Q' : '-',
							   pos->castlePerm & BKCA ? 'k' : '-',
							   pos->castlePerm & BQCA ? 'q' : '-');	// Stampa 4 caratteri per i permessi dell'arrocco ('-' se il corrsipondente permesso non c'e')	

	printf("Poskey:%llX\n",pos->posKey);							// Stampa in esadecimale l'haskey della posizione (llX = long long hexadecimal)
}


