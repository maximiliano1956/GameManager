#include "Defines.h"

//
// Funzione che inizializza il contenuto della hashtable
//
// INPUT:	table			// pointer alla hashtable

void ClearHashTable(S_HASHTABLE *table) {

	int index;				// indice al record corrente
	int numRec;				// numero di entries nella tabella
	S_HASHENTRY *pRec;		// pointer al record corrente

	numRec = table->numEntries;			// Ricava numero di record memorizzabili
	pRec = table->pTable;				// Fa' puntare al primo record (testa del vettore dei record)

	for (index = 0; index < numRec; index++) {		// Loop su tutti i record
	
		pRec->move.nbmoves = 0;						// Nessuna mossa
		pRec->posKey = 0ULL;						// Azzera la hashkey (la rende invalida)
		pRec->depth = 0;							// Azzera la profondita'
		pRec->score = 0;							// Azzera il punteggio
		pRec->flags = HFNONE;						// Assegna valore non valido al flag

		pRec++;										// Passa al record successivo
	}

	table->newWrite = 0;
}


//
// Funzione che alloca la ram per la hashtable e inizializza la struttura di tipo PV_TABLE
// con il corretto numero di entries dichiarato facendo puntare la testa del vettore
// della hashtable alla ram allocata
//
// INPUT:	table		Pointer alla struttura di tipo PV_TABLE da inizializzare
//			MB			Numero di megabytes da allocare

void InitHashTable(S_HASHTABLE *table,int MB) {

	int HashSize;															// Memoria da allocare in bytes

	HashSize = MB * 0x100000;												// 1Mb = 2^20 bytes (20 = 5*4 percio' 5 nibbles)

	table->numEntries = HashSize / sizeof(S_HASHENTRY);						// Calcola quanti record possono essere memorizzati in base alla ram totale e allo spazio necessario per ogni record
	table->numEntries -= 2;													// Per star nel sicuro, lo diminuisce un po' per non stare al pelo con lo sforamento (grezzo...)

	if (table->pTable != NULL)												// Se ram gia' allocata
		free(table->pTable);												// la dealloca

	table->pTable = (S_HASHENTRY *) malloc(HashSize);						// Alloca la memoria e carica il pointer alla testa del vettore

	if (table->pTable == NULL) {											// Test se errore su allocazione ram
	
		DoLog("Hash Allocation Failed, trying %dMB...\n",MB/2);
		InitHashTable(table,MB/2);											// Ritenta con un size dimezzato
	}
	else {

		ClearHashTable(table);														// Inizializza la hashtable
		DoLog("HashTable init complete with %d entries\n",table->numEntries);		// Info su hash allocata
	}
}

//
// Questa funzione cerca un record nella hashtable
//
// INPUT:	hashKey		chiave hash
//			move	pointer alla mossa trovata (out)
//			score	pointer al punteggio della mossa trovata (out)
//			alpha	valore di alfa attuale
//			beta	valore di beta attuale
//			depth	valore di profondita' di ricerca attuale
//
// OUTPUT:	TRUE	record trovato (informazioni valide in out)

int ProbeHashEntry(U64 hashKey,CompMove *move, int *score,int alpha,int beta,int depth) {

	int index;													// Indice nel vettore hashtable dove cercare il record

	index = hashKey % HashTable.numEntries;						// Ricava l'indice in hastable che compete alla posizione

	assert(index >=0 && index < HashTable.numEntries);			// Controllo la correttezza dell'indice ottenuto

	assert(depth >= 1 && depth < MAXDEPTH);						// Controllo correttezza della profondita'

	assert(alpha < beta);										// Controllo sul valore di alfa in relazione a quello di beta

	assert(alpha >= LOSS_SCORE && alpha <= WIN_SCORE);			// Controllo sulla validita' del punteggio alfa
	
	assert(beta >= LOSS_SCORE && beta <= WIN_SCORE);			// Controllo sulla validita' del punteggio beta

	if (HashTable.pTable[index].posKey == hashKey) {			// Controllo se all'index ottenuto e' memorizzata la hashkey della posizione
	
		*move = HashTable.pTable[index].move;					// Okay: la hashkey corrisponde: carica in out la mossa memorizzata (serve per la variante principale)

		if (HashTable.pTable[index].depth >= depth) {			// Se la posizione era stata memorizzata durante una ricerca di profondita' maggiore o uguale a quella attuale, l'informazione della hash e' attendibile
		
			HashTable.hit++;									// Aggiorna counter posizioni attendibile trovate

			assert(HashTable.pTable[index].depth >= 1 && HashTable.pTable[index].depth < MAXDEPTH);		// Controllo sul valore della profondita' memorizzata

			assert(HashTable.pTable[index].flags >= HFALPHA && HashTable.pTable[index].flags <= HFEXACT);	// Controllo sul valore del flag memorizzato

			*score = HashTable.pTable[index].score;		// Carica in out il punteggio memorizzato in hash relativo alla posizione memorizzata

			// Il punteggio memorizzato in hashtable nel caso di punteggio di matto e' WIN_SCORE o LOSS_SCORE (e' stato epurato del contributo della distanza dal matto)
			// Ora corregge lo score in out rimettendo il contributo della distanza del matto

			if (*score > ISMATE)
				*score -= (*pGetPly)();

			if (*score < -ISMATE)
				*score += (*pGetPly)();
					
			assert(*score >= LOSS_SCORE && *score <= WIN_SCORE);			// Controllo sul valore di score ottenuto 

			switch (HashTable.pTable[index].flags) {						// Ora in base al flag forse bisognera' correggere lo score


				case	HFALPHA:		if (*score <= alpha) {				// Il punteggio memorizzato e' in realta l'alfa della ricerca e se esso e' superiore all'attuale alfa l'informazione in hash non puo' essere utilizzata

											*score = alpha;					// In caso contrario esce con l'alfa attuale come punteggio
											return TRUE;
										}

										break;

				case	HFBETA:			if (*score >= beta) {				// Il punteggio memorizzato e' in realta il beta della ricerca (c'e' stato cutoff) e se esso e' inferiore all'attuale beta l'informazione in hash non puo' essere utilizzata

											*score = beta;					// In caso contrario esce con in beta attuale come punteggio
											return TRUE;
										}

										break;

				case	HFEXACT:		return TRUE;						// Se il punteggio era esatto lo ha gia' caricato in out : non lo deve limitare

										break;

				default:				assert(FALSE);						// Valore del campo flags scorretto
										break;
			}
		}
	}

	// L'informazione in hash non e' fornita in uscita

	return FALSE;			
}


//
// Funzione che memorizza un nodo della ricerca nella hashtable
//
// INPUT:	hashKey		chiave hash
//			move		mossa da memorizzare codificata in un intero
//			score
//			flags
//			depth

void StoreHashEntry(U64 hashKey,CompMove move,int score,int flags,int depth) {

	int index;													// Indice nel vettore hashtable dove verra' memorizzato il record

	index = hashKey % HashTable.numEntries;					// Ricava l'indice in hastable che compete alla posizione

	assert(index >=0 && index < HashTable.numEntries);			// Controllo la correttezza dell'indice ottenuto

	assert(depth >= 1 && depth < MAXDEPTH);						// Controllo correttezza della profondita' da memorizzare

	assert(flags >= HFALPHA && flags <= HFEXACT);				// Controllo sulla correttezza del flag da memorizzare

	assert(score >= LOSS_SCORE && score <= WIN_SCORE);			// Controllo sul valore di score da memorizzare

	if (HashTable.pTable[index].posKey == 0)					// Controllo se la locazione e' vuota o gia' occupata da un dato memorizzato in precedenza
		HashTable.newWrite++;									// Aggiorna counter nuova scrittura
	else
		HashTable.overWrite++;									// Aggiorna il counter informazione sovrascritta

	if (score > ISMATE)
		score = WIN_SCORE;

	if (score < -ISMATE)
		score = LOSS_SCORE;

	// Memorizza le varie informazioni nel record

	HashTable.pTable[index].posKey = hashKey;				// Chiave hash
	HashTable.pTable[index].move = move;					// Mossa codificata in un intero
	HashTable.pTable[index].score = score;					// Punteggio
	HashTable.pTable[index].flags = flags;					// Flag
	HashTable.pTable[index].depth = depth;					// Profondita' raggiunta nella posizione
}


//
// Funzione che ottiene una mossa dalla hashtable
//
// INPUT:	hashKey	chiave hash
//			move	pointer mossa in out
// OUTPUT:	FALSE	se la mossa non e' stata trovata

int ProbePvMove(U64 hashKey, CompMove *move) {

	int index;																// Indice nel vettore hashtable dove verra' memorizzato il record

	index = hashKey % HashTable.numEntries;									// Ricava l'indice in hastable che compete alla posizione

	assert(index >=0 && index < HashTable.numEntries);						// Controllo la correttezza dell'indice ottenuto

	if (HashTable.pTable[index].posKey == hashKey) {						// Se l'hashkey completa corrisponde...
		
		*move = HashTable.pTable[index].move;								// Ricava la mossa memorizzata

		return TRUE;														// Mossa trovata
	}

	return FALSE;															// In caso contrario non c'e' nessuna mossa memorizzata corripsondente alla posizione in input
}

//
// Funzione che carica nel vettore PvArray della posizione
// la variante principale fino ad un massimo di profondita'
// Normalmente questa funzione viene chiamata alla fine di una ricerca
// Le mosse della variante vengono ricavate dalla hashtable
//
// INPUT:	depth			profondita' massima della variante
// OUTPUT:					profondita' della variante principale ottenuta
//

int GetPvLine(int depth) {

	CompMove move;					// Mossa corrente
	char moveBuffer[MAXLMOVE];
	DLL_Result esito;

	assert(depth < MAXDEPTH);		// Controlla che il massimo depth in input non ecceda il massimo consentito

	PvArray.nel = 0;							// Init profondita' mossa in variante

	while ( ProbePvMove((*pGetHashKey)(),&move)  && PvArray.nel < depth) {		// Continua a ricavare mosse dalla hashtable finche' ce ne sono e finche' non si raggiunge la profondita' massima impostata
	
		assert(PvArray.nel < MAXDEPTH);			// Eccesso di sicurezza...

		if (!MoveExists(move))				// Se la mossa e' illegale c'e' stata una collisione; interrompe la ricerca
			break;

		strcpy(moveBuffer,move.str_moves);

		esito = DoMove(moveBuffer);	// Esegue la mossa

		assert(esito == DLL_OK);							// Controllo se mossa corretta

		PvArray.Pv[PvArray.nel++] = move;		// Memorizza la mossa nella variante principale e aggiorna la profondita'
	}

	// Finito il caricamento della variante principale deve tornare indietro

	while ( (*pGetPly)() > 0 )
		(*pTakeBack)();					// Ricordiamo che MakeMoveV se esegue la mossa incrementa il ply mentre TakeMove lo decrementa

	return PvArray.nel;
}
