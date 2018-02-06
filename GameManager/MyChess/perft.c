#include "defs.h"


// Implementazione estesa del perft


//
// struttura utilizzata nel Perft esteso
//

typedef struct {
	char fen[256];					// Posizione fen di test
	U64 targetLeafNodes;			// Numero di "foglie" dichiarato nel database per una certa profondita'
	U64 leafnodes;					// Nr. "foglie" come risultato dell'esplorazione dell'albero delle mosse alla profondita' voluta
	U64 nodes;						// Nr. di posizioni totali esplorate fino alla profondita' voluta
	long startTime;					// Istante inizio ricerca
	long endTime;					// Istante fine ricerca
	long savedNodes;				// ??
} S_PERFTRES;


//
// Inizializza la struttura per il Perft esteso
//
// INPUT:	res		pointer alla struttura da resettare

void ResetResult(S_PERFTRES *res) {

	memset(&res->fen[0],256,0);		// Azzera il buffer per la posizione in formato fen
	res->targetLeafNodes = 0;		// Azzera .... tutto..
	res->leafnodes = 0;
	res->nodes = 0;
	res->startTime = 0;
	res->endTime = 0;
	res->savedNodes = 0;
}


//
// Del tutto simile all'originale Perft
// L'unica differenza e' che invece di appogiarsi
// alla variabile statica esterna, utilizza
// una struttura il cui pointer viene fornito in input
// NOTA: La funzione e' ricorsiva
//
// INPUT:	depth			profondita' ricerca
//			pos				pointer alla posizione di partenza
//			res				pointer alla struttura statica esterna

void PerftGo(int depth,S_BOARD *pos,S_PERFTRES *res) {

	S_MOVELIST list;			// Lista mosse possibile nella posizione corrente
	int MoveNum;				// Indice esplorazione lista mosse possibili

	assert(CheckBoard(pos));	// Controlla coerenza posizione iniziale

	if (depth == 0) {			// Controllo raggiungimento "foglia" dell'albero delle mosse
		res->leafnodes++;		// In tal caso incrementa il counter delle "foglie"
		return;					// ed esce
	}

	GenerateAllMoves(pos,&list);	// Genera tutte le mosse possibili a partire dalla posizione pos

	for (MoveNum = 0; MoveNum < list.count; MoveNum++)		// Loop su tutte le mosse possibili
		if (MakeMoveV(pos,list.moves[MoveNum].move)) {		// Se la mossa e' legale
		
			PerftGo(depth - 1,pos,res);						// Ricorsione a un depth inferiore di 1
			TakeMove(pos);									// Ritorna alla posizione precedente
			res->nodes++;									// Aggiorna counter posizioni esplorate
		}
}


//
// Del tutto simile all'originale PerftTest
// L'unica differenza e' che invece di appoggiarsi
// a variabili statiche esterne, utilizza
// una struttura il cui pointer viene fornito in input
// Non chiama la funzione Pertf() ma la PerftGo()
//
// INPUT:	depth			profondita' ricerca
//			res				pointer alla struttura statica esterna

void PerftGoRoot(int depth,S_PERFTRES *res) {

	S_MOVELIST list;					// Variabile in cui verra' scaricata la lista delle possibili mosse della posizione
	S_BOARD	pos;						// Posizione iniziale
	int MoveNum;						// Indice nella lista delle mosse possibili
	int move;							// Mossa al primo livello (root)
	U64 cumnodes;						// Valore di leafNodes prima di eseguire una mossa a livello di root
	U64 oldnodes;						// Numero di "foglie" raggiunte dall'ultima mossa a livello di root esplorata

	ParseFen(res->fen,&pos);			// Carica la posizione fen

	assert(CheckBoard(&pos));			// Controllo coerenza posizione iniziale pos

	GenerateAllMoves(&pos,&list);		// Genera tutte le mosse possibili nella posizione pos

	for (MoveNum = 0; MoveNum < list.count; MoveNum++) {		// Loop su tutte le mosse possibili
		move = list.moves[MoveNum].move;						// Mossa corrente
		if (MakeMoveV(&pos,move)) {								// Se la mossa e' legale
		
			cumnodes = res->leafnodes;							// Tiene copia del numero di "foglie" prima di esplorare la mossa

			PerftGo(depth - 1,&pos,res);						// Imposta un'altra esplorazione a partire dalla nuova
																// posizione indicando una profondita' target diminuita di 1
			TakeMove(&pos);										// Torna alla posizione originale, pronto per esplorare
																// la posizione generata dalla prossima mossa della lista
			res->nodes++;										// Aggiorna numero di posizioni esplorate
			oldnodes = res->leafnodes - cumnodes;				// Valuta quante "foglie" si sono esplorate nell'ultima mossa di root
			DoLog("move %d : %s : %llu %llu %llu\n",MoveNum + 1,PrMove(move),oldnodes,res->leafnodes,res->targetLeafNodes);	// Stampa info mossa a livello di root:
																				// Numero mossa, mossa, nr. "foglie" esplorate
																				// in questo ramo di root dell'albero delle mosse
		}
	}
}

//
// Decodifica una linea del database del Perft
// relativamente alla profondita' indicata
// e depone le informazioni nella struttura
// il cui pointer e' fornito in input
//
// Ricordiamo che una linea del database e' del tipo;
// 4k3/8/8/8/8/8/8/4K2R w K - 0 1 ;D1 15 ;D2 66 ;D3 1197 ;D4 7059 ;D5 133987 ;D6 764643
// Come si vede si ha la posizione fen e poi tanti campi del tipo: ";Dx y"
// dove x e' la profondita' e y e' il numero di "foglie" da ottenere
//
// INPUT:	linea		Linea del database Perft() da decodificare
//			res			Pointer struttura delle info decodificate
//			depth		Profondita' target

void ParsePerftLine(char *line,S_PERFTRES *res,int depth) {

	int index;						// Indice nella linea da decodificare
	char c;							// Carattere nella linea

	ResetResult(res);				// Inizializza struttura di appoggio

	index = 0;

	while ((c = *line++) != ';')
		res->fen[index++] = c;		// Carica la linea fen all'inizio fino al carattere ';'

	res->fen[index] = '\0';			// Inserisce carattere fine stringa					

	while ((c = *line++) != '\0')	// Prosegue eventualmente fino al carattere di fine stringa
		if (c == 'D' && atoi(line) == depth) {	// Ricerca il carattere 'D'; e se la profondita' indicata corrisponde a quella in input
			line += 2;			// Si suppone che il dato di profondita' stia su 2 caratteri (es: "D2 ")
#ifdef WIN32
			res->targetLeafNodes = _strtoui64(line,NULL,10);		// Ricava il numero di "foglie" da ottere
#else
			res->targetLeafNodes = strtoull(line,NULL,10);			// Ricava il numero di "foglie" da ottere
#endif
			DoLog("Fen: %s : Target %llu\n",res->fen,res->targetLeafNodes);		// Stampa info decodificate
			return;				// ed esce
		}
}

//
// Funzione che che realizza il Perft esteso, ma non legge il file "testsuite.epd"
// Invece una linea nel formato di testsuite.epd deve essere fornita in ingresso
//
// INPUT:	depth				Profondita' voluta
//			fenLineWithDepth	Linea del database Perft

void PerftOne(int depth,char *fenLineWithDepth) {

	S_PERFTRES res;				// Struttura di appoggio della ricerca
	int success;				// FALSE/TRUE	Valore del risultato del test
								// TRUE se il numero di "foglie" corrisponde

	ResetResult(&res);			// Init struttura di appoggio

	DoLog("\n\n****************** NEW PERFT, Depth %d ******************\n",depth);	// Stampa profondita' del test
	ParsePerftLine(fenLineWithDepth, &res,depth);										// Ricava le info dalla linea del database
	DoLog("After Parse: Target: %llu Actual %llu\n",res.targetLeafNodes,res.leafnodes);	// Stampa nr. foglie da ottenere
	PerftGoRoot(depth,&res);															// Esegue la ricerca
	DoLog("After Perft: Target: %llu Actual %llu\n",res.targetLeafNodes,res.leafnodes);	// Stampa nr. foglie ottenute
	success = res.leafnodes == res.targetLeafNodes ? TRUE : FALSE;						// Fa' il confronto
	DoLog("\nResult: %s : %s  leaf: %llu   target: %llu\n",res.fen,
		(success) ? "OK" : "**** FAILED ****",res.leafnodes,res.targetLeafNodes);		// Stampa risultato
}

//
// Perft() esteso con info ricavate dal file "testsuite.epd"
// che deve essere presente nello stesso direttorio dell'eseguibile
//
// INPUT:	depth			Profondita' del test

void PerftFile(int depth) {

	FILE *perftFile;			// Descrittore file "testsuite.epd"
	char lineIn[1024];			// Buffer per una linea; deve poter contenere una singola linea del file
	S_PERFTRES results[512];	// Vettore contenente le varie strutture di appoggio di ogni test; si suppone un massimo di 512 test
								// Quindi "testsuite.epd" non potra' contenere piu' di 512 linee 
	int resCount;				// Nr. test decodificati
	int index;					// Indice esplorazione risultati del test
	int success;				// Risultato test corrente
	int	errfile;			// Codice errore in apertura file

	resCount = 0;				// Azzera numero tes da eseguire
#ifdef _MSC_VER
	errfile = (int)fopen_s(&perftFile,"perftsuite.epd","r");		// Ricava descrittore del file "testsuite.epd"

	if (errfile != (int)NULL) {						// Test se errore in apertura file
		DoLog("Open file error: %d\n",(int)errfile);
#else
	perftFile = fopen("perftsuite.epd","r");		// Ricava descrittore del file "testsuite.epd"

	if (perftFile == NULL) {
		DoLog("Open file error\n");
#endif
		return;

	} else
		while (fgets(lineIn,1024,perftFile) != NULL)				// Legge una linea
			ParsePerftLine(lineIn,&results[resCount++],depth);		// Decodifica la linea

	fclose(perftFile);												// Chiude il file database del Perft

	if (resCount)													// Se almeno un test da eseguire
		DoLog("\n\nRESULTS: \n\n");

	for (index = 0; index < resCount; index++)
		PerftGoRoot(depth,&results[index]);							// Esegue resCount tests

	for (index = 0; index < resCount; index++) {					// Valutazione dei tests
		success = results[index].leafnodes == results[index].targetLeafNodes ? TRUE : FALSE;	// Ricava se il risultato e' giusto
		DoLog("%s : %s leafNodes=%llu target=%llu\n", (success) ? "OK" : "FAILED",
				results[index].fen,results[index].leafnodes,results[index].targetLeafNodes);	// Stampa risultato
	}

	DoLog("\n\nNumero tests = %d\n",resCount);
}


// Implementazione originale del Perft


U64 leafNodes;			// Uscita della funzione ricorsiva Perft


//
// Funzione che esplora l'albero delle mosse della posizione fino a una
// certa profondita'
// Nella variabile statica leafNodes viene depositato il numero di
// posizioni raggiungibili alla profondita' impostata.
// In pratica ricava il numero di "foglie" dell'albero alla profondita' prefissata
// La funzione e' ricorsiva; la variabile leafnodes va' chiaramente azzerata
// dal software esterno prima di chiamare la funzione Perft()
//
// INPUT:	depth	profondita' di esplorazione dell'alvero delle mosse
//			pos		pointer alla posizione

void Perft(int depth,S_BOARD *pos) {

	S_MOVELIST list;					// Variabile in cui verra' scaricata la lista delle possibili mosse della posizione
	int MoveNum;						// Indice nella lista delle mosse possibili

	assert(CheckBoard(pos));			// Controllo coerenza della posizione data

	if (depth == 0) {					// Se si e' raggiunto la profondita' voluta (cioe' una "foglia" dell'albero)
	
		leafNodes++;					// Incrementa il counter delle "foglie"
		return;							// ed esce dall'esplorazione
	}

	GenerateAllMoves(pos,&list);		// Genera tutte le mosse possibili nella posizione

	for (MoveNum = 0; MoveNum < list.count; MoveNum++)			// Esplora tutte le mosse in lista

		if (MakeMoveV(pos,list.moves[MoveNum].move))	{		// Esegue la mossa se legale
			
			Perft(depth - 1,pos);								// In tal caso imposta un'altra esplorazione a partire dalla nuova
																// posizione indicando una profondita' target diminuita di 1
			TakeMove(pos);										// Torna alla posizione originale, pronto per esplorare
																// la posizione generata dalla prossima mossa della lista
		}
}


//
// Questa funzione ricava da una posizione iniziale (root position)
// per ogni mossa possibile (a livello di root appunto)
// quante "foglie" verranno raggiunte nell'esplorazione fino alla profondita' voluta
// Chiaramente la somma delle varie "foglie" di ogni mossa a livello di root
// eguagliera' il totale delle foglie dell'albero completo
//
// INPUT:	depth	profondita' di esplorazione dell'alvero delle mosse
//			pos		pointer alla posizione

void PerftTest(int depth,S_BOARD *pos) {

	S_MOVELIST list;					// Variabile in cui verra' scaricata la lista delle possibili mosse della posizione
	int MoveNum;						// Indice nella lista delle mosse possibili
	int move;							// Mossa al primo livello (root)
	U64 cumnodes;						// Valore di leafNodes prima di eseguire una mossa a livello di root
	U64 oldnodes;						// Numero di "foglie" raggiunte dall'ultima mossa a livello di root esplorata
	int start;							// Tempo iniziale del test

	assert(CheckBoard(pos));			// Controllo coerenza della posizione data

	PrintBoard(pos);					// Stampa la posizione iniziale

	DoLog("\nStarting test to depth: %d\n",depth);

	start = GetTimeMs();				// Fotografa il tempo iniziale

	leafNodes = 0;						// Doveroso frima di chiamare la funzione Pertf()

	GenerateAllMoves(pos,&list);		// Genera tutte le mosse possibili nella posizione

	for (MoveNum = 0; MoveNum < list.count; MoveNum++) {		// Esplora tutte le mosse in lista

		move = list.moves[MoveNum].move;						// Estrae la prossima mossa dalla lista

		if (MakeMoveV(pos,move)) {								// Esegue la mossa se legale

			cumnodes = leafNodes;								// Tiene copia del numero di "foglie" prima di esplorare la mossa

			Perft(depth - 1,pos);								// Imposta un'altra esplorazione a partire dalla nuova
																// posizione indicando una profondita' target diminuita di 1
			TakeMove(pos);										// Torna alla posizione originale, pronto per esplorare
																// la posizione generata dalla prossima mossa della lista

			oldnodes = leafNodes - cumnodes;					// Valuta quante "foglie" si sono esplorate nell'ultima mossa

			DoLog("move %d : %s : %ld\n",MoveNum + 1,PrMove(move),oldnodes);
		}
	}

	DoLog("\nTest Complete : %ld nodes visited",leafNodes);
	DoLog(" in %d ms\n",GetTimeMs() - start);					// Stampa anche il tempo impiegato nella ricerca (l'ho fatto su 2 linee poiche' se fatto nella linea precedente stampava sempre 0! BOOOOH!)
}