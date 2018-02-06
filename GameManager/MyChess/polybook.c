#include "defs.h"
#include "polykeys.h"

#define	MAXBOOKMOVES	32			// Numero massimo mosse presnti in libreria ed associate alla stessa posizione


//
// Questa struttura rappresenta una entry nel database delle aperture in formato polyglot
//

typedef struct {

	U64	key;						// Key a 64 bit
	unsigned short move;			// Mossa codificata su 16 bit
	unsigned short weight;			// Valore della mossa su 16 bit
	unsigned int learn;				// Per l'autoapprendimento (su 32 bit) (non verra' usato)

} S_POLY_BOOK_ENTRY;


long NumEntries;					// Numero di entries nel book

S_POLY_BOOK_ENTRY *entries;			// Pointer al book CHE VERRA' CARICATO INTERAMENTE IN RAM
									// Per questo motivo la libreria della aperture dovra' essere "leggera"
									// Per gestire librerie delle aperture voluminose BISOGNA CAMBIARE GESTIONE
									// Insomma .. sulla gestione si e' cercato la soluzione piu' semplice...

//
// Questa macro ricava l'indice di accesso alla tabella delle keys
// in base al codice pezzo p, alla traversa r e alla colonna f

#define OFFSET_POLY(p,f,r) ( 64 * (p) + 8 * (r) + f )


//
// Questa tabella serve per convertire il codice
// pezzo nel nostro formato al codice pezzo nel
// formato polyglot per accedere alla tabella delle keys
// Si entra avendo come indice il codice del pezzo

int PolyKindOfPiece[] = {

	-1, 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10
};

int UseBook;

//
// Questa funzione carica in ram la libreria delle aperture
// e inizializza le variabili di gestione
//

void IniPolyBook(void) {

	FILE *pFile;			// Pointer al file
	int position;			// Displacement nel file
	int returnValue;		// Nr. bytes effettivamente letti dal file con fread()

	UseBook = FALSE;

	entries = NULL;			// Per default la rma non e' stata ancora allocata

	// Il file del book si deve chiamare "performance.bin"

#ifdef _MSC_VER

	if (fopen_s(&pFile,"performance.bin","rb"))				// Cerca di aprire il file
#else
	if ((pFile = fopen("performance.bin","rb")) == NULL)	// Cerca di aprire il file
#endif
		DoLog("Book file not found!\n");
	else {
	
		fseek(pFile,0,SEEK_END);			// Si posiziona alla fine del file
	
		position = ftell(pFile);			// Ricava il displacement della posizione attuale nel file: ora posizion contiene il numero dei bytes del file

		if (position < sizeof(S_POLY_BOOK_ENTRY)) {		// Test se almeno un record presente
	
			DoLog("No entries found in book\n");
			return;
		}

		NumEntries = position / sizeof(S_POLY_BOOK_ENTRY);		// Ricava il numero dei record presenti nel book
	
		DoLog("%ld Entries found in book\n",NumEntries);		// Stampa il numero dei record presenti nel book

		entries = (S_POLY_BOOK_ENTRY *)malloc(NumEntries * sizeof(S_POLY_BOOK_ENTRY));		// Alloca la ram e inizializza il puntatore (se ram insufficiente viene caricato NULL)

		if (entries == NULL) {									// Controllo se ram allocata

			DoLog("Not enough memory for book reading!\n");
			return;
		}

		rewind(pFile);																	// Si riporta all'inizio del file

		returnValue = (int)fread(entries,sizeof(S_POLY_BOOK_ENTRY),NumEntries,pFile);	// Prova a leggere dal file NumEntries record (in out returnValue rappresenta il nr. di record effettivamente letti)

		DoLog("fread() %ld Entries Read in from file\n",returnValue);					// Stampa il br. record letti (che sara' <= NumEntries)

		fclose(pFile);																	// Chiude il file

		if (returnValue > 0) {
			UseBook = TRUE;																// Se trovato almeno un entry attiva l'utilizzo della libreria
			NumEntries = returnValue;													// Dichiara il numero di entries caricati in ram
		}
	}
}


//
// Questa funzione dealloca la ram della libreria delle aperture
//

void CleanPolyBook(void) {

	if (entries)					// Se ram allocata
		free(entries);				// Dealloca la ram
}


//
// Questa funzione controlla che nel caso
// la casella en-passant sia attiva ci sia
// effettivamente al turno successivo un pedone
// nella posizione tale da potere effettuare realmente
// la presa en-passant
//
// OUTPUT:	TRUE		la presa en-passant e' effettivamente possibile

int HasPawnForCapture(void) {

	int sq;						// Casella en-passant
	int sqPCap;					// Casella dove deve essere presente il pedone che fa' la cattura

	sq = pos->enPas;

	if (sq == NO_SQ)
		return FALSE;			// Solo se c'e' effettivamente una casella en-passant attiva la presa sara' eventualmente possibile

	if (pos->side == WHITE) {

		// Deve muovere il Bianco
	
		sqPCap = sq - 11;		// Il Pedone Bianco si deve trovare nella traversa inferiore ( - 10) e puo' essere a sinistra (ulteriore - 1)

		if (sqPCap != OFFBOARD)					// Solo se la casella calcolata e' sulla scacchiera reale
			if (pos->pieces[sqPCap] == wP)		// Test se sulla casella calcolata e' presente un Pedone Bianco
				return TRUE;

		sqPCap = sq - 9;		// Il Pedone Bianco si deve trovare nella traversa inferiore ( - 10) e puo' essere a destra (ulteriore + 1)

		if (sqPCap != OFFBOARD)					// Solo se la casella calcolata e' sulla scacchiera reale
			if (pos->pieces[sqPCap] == wP)		// Test se sulla casella calcolata e' presente un Pedone Bianco
				return TRUE;
	} else {

		// Deve muovere il Nero
	
		sqPCap = sq + 9;		// Il Pedone Nero si deve trovare nella traversa superiore ( + 10) e puo' essere a sinistra (ulteriore - 1)

		if (sqPCap != OFFBOARD)					// Solo se la casella calcolata e' sulla scacchiera reale
			if (pos->pieces[sqPCap] == bP)		// Test se sulla casella calcolata e' presente un Pedone Nero
				return TRUE;

		sqPCap = sq + 11;		// Il Pedone Nero si deve trovare nella traversa superiore ( + 10) e puo' essere a destra (ulteriore + 1)

		if (sqPCap != OFFBOARD)					// Solo se la casella calcolata e' sulla scacchiera reale
			if (pos->pieces[sqPCap] == bP)		// Test se sulla casella calcolata e' presente un Pedone Nero
				return TRUE;
	}

	return FALSE;				// La presa en-passant non e' possibile
}


//
// Questa funzione calcola la hashkey di una posizione
// nel formato polyglot
//
// OUTPUT:					chiave hash nel formato polyglot

U64 PolyKeyFromBoard(void) {

	int sq;				// Casella nella scacchiera estesa
	U64 finalKey;		// Hashkey nel formato polyglot
	int piece;			// Codice pezzo nella casella

	finalKey = 0;		// Init hashkey da calcolare

	// Contributo del pezzo nella casella

	for (sq = 0; sq < BRD_SQ_NUM; sq++) {	// Loop sulle casella della scacchiera estesa
	
		piece = pos->pieces[sq];			// Ricava codice pezzo nella casella

		if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {		// Se il codice pezzo e' relativo a una casella valida non vuota
		
			assert(piece >= wP && piece <= bK);								// Controlla la validita' del codice pezzo

			finalKey ^= Random64Poly[OFFSET_POLY(PolyKindOfPiece[piece],FilesBrd[sq],RanksBrd[sq])];		// In base al codice pezzo e alla casella accede alla matrice delle keys e fa' ex-or con la key trovata.	
		}
	}

	// Contributo dei permessi dell'arrocco
	// Le 4 keys per l'arrocco sono nella matrice alle locazioni 768,769,770,771

	if (pos->castlePerm & WKCA)
		finalKey ^= Random64Poly[768];
	if (pos->castlePerm & WQCA)
		finalKey ^= Random64Poly[769];
	if (pos->castlePerm & BKCA)
		finalKey ^= Random64Poly[770];
	if (pos->castlePerm & BQCA)
		finalKey ^= Random64Poly[771];

	// Contributo casella en-passant
	// La key base e' presente nella matrice alla locazione 772: poi a questa key viene aggiunto la colonna della casella en-passant, ottenendo le keys 772,773,774,775,776,777,778,779 (colonne a,b,c,d,e,f,g,h)

	if (HasPawnForCapture())											// Nel formato polyglot la casella en-passant contribuisce alla haskey SOLO se la presa e' effettivamente possibile
		finalKey ^= Random64Poly[772 + FilesBrd[pos->enPas]];

	// Contributo della parte che deve muovere
	// La key e' presente nella matrice alla locazione 780

	if (pos->side == WHITE)
			finalKey ^= Random64Poly[780];

	return finalKey;
}


//
// Le 3 funzioni che segueno sono state importate da internet
// Esse invertono l'ordine dei bytes per interi a 16,32 e 64 bit
//
// Servono per passare dal formato little-endian a big-endian e viceversa


unsigned short endian_swap_u16(unsigned short x) 
{ 
    x = (x>>8) | 
        (x<<8); 
    return x;
} 

unsigned int endian_swap_u32(unsigned int x) 
{ 
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) | 
        ((x>>8) & 0x0000FF00) | 
        (x<<24); 
    return x;
} 

U64 endian_swap_u64(U64 x) 
{ 
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) | 
        ((x<<24) & 0x0000FF0000000000) | 
        ((x<<8)  & 0x000000FF00000000) | 
        ((x>>8)  & 0x00000000FF000000) | 
        ((x>>24) & 0x0000000000FF0000) | 
        ((x>>40) & 0x000000000000FF00) | 
        (x<<56); 
    return x;
}


//
// Questa funzione converte la mossa dal formato polyglot
// a quello interno
//
// INPUT:	move	mossa nel formato polyglot
// OUTPUT:			mossa codificata nel formato interno (NOMOVE ( = 0) se mossa non valida nella posizione)
//

int ConvertPolyToInternalMove(int move) {

	int	ff;			// Colonna casella di partenza
	int fr;			// Traversa casella di partenza
	int tf;			// Colonna casella di arrivo
	int tr;			// Traversa casella di arrivo
	char MoveString[6];		// Notazione algebrica della mossa da convertire (es: e7e8q quindi 5 char + fine stringa)

	int pp;			// Codice pezzo promosso

	// Ricava caselle di partenza e arrivo

	ff = (move >> 6) & 0x07; 
	fr = (move >> 9) & 0x07;
	tf = move & 0x07;
	tr = (move >> 3) & 0x07;
#ifdef _MSC_VER
	sprintf_s(MoveString,6,"%c%c%c%c",FileChar[ff],RankChar[fr],FileChar[tf],RankChar[tr]);		// Crea la stringa algebrica senza amcora l'eventuale carattere del pezzo promosso
#else
	sprintf(MoveString,"%c%c%c%c",FileChar[ff],RankChar[fr],FileChar[tf],RankChar[tr]);		// Crea la stringa algebrica senza amcora l'eventuale carattere del pezzo promosso
#endif

	pp = (move >> 12) & 0x07;		// Ricava codice pezzo promosso

	switch (pp) {

		case 1:		MoveString[4] = 'n';		// Pezzo promosso = Cavallo
					break;
		case 2:		MoveString[4] = 'b';		// Pezzo promosso = Alfiere
					break;
		case 3:		MoveString[4] = 'r';		// Pezzo promosso = Torre
					break;
		case 4:		MoveString[4] = 'q';		// Pezzo promosso = Regina
					break;
		default:	MoveString[4] = 'q';		// Default se codice non riconosciuto
					break;
	}

	if (pp > 0)							// Se c'e' un pezzo promosso
		MoveString[5] = '\0';			// aggiunge alla fine della stringa il carettere di fine stringa

	return ParseMove(MoveString);		// Verifica se la mossa e' corretta e la codifica	
}


//
// Questa funzione restituisce una mossa dalla libreria
// delle aperture relativa alla posizione in input
// Se ci sono piu' mosse ne viene scelta una in maniera casuale
//
// IL formato della mossa codificata da polyglot e' il seguente:
//
// bit	0,1,2		colonna casella di partenza
//		3,4,5		traversa casella di partenza
//		6,7,8		colonna casella di arrivo
//		9,10,11		traversa casella di arrivo
//
// OUTPUT:					mossa codificata
//

CompMove GetBookMove(void) {

	int n_entry;					// Numero della entry nel book
	int move;						// Mossa trovata
	int tempMove;					// Mossa trovata convertita nel formato di vice
	int bookMoves[MAXBOOKMOVES];	// Lista delle mosse trovate nella libreria delle apertura ed associate alla posizione
	int nmoves;						// Nr. mosse depositate nella lista
//	int index;						// Indice per la stampa delle mosse
	U64 polyKey;					// La hashkey nel formato polyglot della posizione attuale
	CompMove cmove;					// mossa trovata nel formato esterno

	cmove.nbmoves = 0;				// Default nessuna mossa trovata

	if (!UseBook)
		return cmove;														// Test se libreria presente

	polyKey = PolyKeyFromBoard();											// Ricava la hashkey in formato polyglot della posizione

	tempMove = NOMOVE;														// Per default nessuna mossa trovata

	nmoves = 0;																// All'inizio la lista bookMoves e' vuota

	for (n_entry = 0; n_entry < NumEntries; n_entry++)						// Loop su tutte le entry del book
	
		if (polyKey == endian_swap_u64(entries[n_entry].key)) {				// La key memorizzata nel book e' nel formato big-endian percio' per passare al little-endian bisogna invertire l'ordine degki 8 bytes
		
			move = endian_swap_u16(entries[n_entry].move);					// Chiave trovata: preleva la mossa convertendola il formato little-endian (inversione dei 2 bytes)

		//	DoLog("Key: %llx Index: %d Move: %c%c%c%c\n",polyKey,n_entry,FileChar[(move >> 6) & 0x07],RankChar[(move >> 9) & 0x07],FileChar[move & 0x07],RankChar[(move >> 3) & 0x07]);

			tempMove = ConvertPolyToInternalMove(move);						// Prova a convertire la mossa nel formato interno

			if (tempMove != NOMOVE) {										// Se la conversione e' avvenuta correttamente
			
				bookMoves[nmoves++] = tempMove;								// Salva la mossa nella lista incrementando l'indice di accesso
				
				if (nmoves >= MAXBOOKMOVES)									// Se la lista e' piena esce dal loop delle entry del book
					break;
			}
		}

	if (nmoves > 0)															// Se e' stata trovata almeno una mossa
		tempMove = bookMoves[rand() % nmoves];								// Sceglie in maniera random

	return (tempMove == NOMOVE) ? cmove : ConvMove(tempMove);				// Converte nel formato esterno

//	DoLog("Listing Book Moves: \n");

//	for (index = 0; index < nmoves; index++)								// Stampa le mosse della lista
//		DoLog("BookMove: %d %s\n",index + 1,PrMove(bookMoves[index]));

}