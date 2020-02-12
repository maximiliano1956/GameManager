#include "defs.h"


//
// Questa funzione viene chiamata quando la GUI invia il comando "position"
//
// Ricordiamo come e' composto il comando:
//
//	a)		position startpos										Inizializza la posizione a quella di partenza
//  b)		position startpos moves e2e4 e7e5 g8f6 etc...			Inizializza la posizione a quella di partenza ed esegui la lista di mosse che segue la stringa moves
//	c)		position fen rbq1b....									Inizializza la posizione a quella indicata nella stringa che segue la stringa fen
//	d)		position fen moves e2e4 e7e5 g8f6 etc...				Inizializza la posizione a quella indicata nella stringa fen che segue la stringa position ed esegui la lista di mosse che segue la stringa moves
//
// INPUT:	lineIn		comando ricevuto ("position...")
//

void ParsePosition(char *lineIn) {

	char *ptrChar;			// Pointer corrente alla stringa in input
	int move;				// Una generica mossa decodificata dall'argomento "moves"

	lineIn += 9;			// Salta la stringa iniziale del comando ( "position "); ora LineIn punta al primo argomento del comando

	if (!strncmp(lineIn,"startpos",8))			// Se comando "position startpos"
		ParseFen(START_FEN);					// Imposta la posizione iniziale
	else {

		ptrChar = strstr(lineIn,"fen");			// Se l'argomento non e' "startpos" ricerca dal primo argomento l'eventuale argomento "fen" (chissa' perche' in tal caso e' stato elastico nel parsing...)

		if (ptrChar == NULL)					// Se stringa "fen" non trovata non e' corretto
			ParseFen(START_FEN);				// In tal caso imposta comunque la posizione iniziale
		else {

			ptrChar += 4;						// Salta i 4 caratteri di "fen " e fa' puntare alla effettiva stringa fen
			ParseFen(ptrChar);					// Imposta la posizione fen voluta
		}

	}

	// Ora bisogna decodificare l'eventuale appendice moves .....

	ptrChar = strstr(lineIn,"moves");			// Ricerca la stringa "moves" a partire dal primo argomento

	if (ptrChar) {								// Se trovata la stringa "moves"
	
		ptrChar += 6;							// Salta "moves "; ora ptrChar punta ai sottoargomenti della sottostringa "moves"

		while (*ptrChar) {						// Continua fino a che non incontra un carattere '\0' ( fine stringa inpostato dalle fgets() )
		
			move = ParseMove(ptrChar);			// Decodifica mossa

			if (move != NOMOVE) {				// Se formato corretto
			
				MakeMoveV(move);				// Esegue la mossa

				pos->ply = 0;					// Poiche' MakeMove incrementa ply

				while (*ptrChar && *ptrChar++ != ' ');			// Cerca (fino ad eventuale fine stringa) il primo carattere diverso da blanks (insomma ora ptrChar punta al '\0' o alla prossima mossa)
			}
			else
				break;							// Se formato mossa errato esce dal parsing delle mosse
		}
	}

	PrintBoard();									// Stampa la posizione impostata
}


//
// Questa funzione viene chiamata quando la GUI invia il comando "go"
// Vediamo ora esempi con vari argomenti:
// go depth n			Start di una ricerca avendo non un limite di tempo ma solo un limite di profondita' che viene settato a n	
// go wtime t			Se deve muovere il Bianco, questo argomento imposta il tempo t assegnato alla ricerca per eseguire le prossime movestogo mosse
// go btime t			Se deve muovere il Nero, questo argomento imposta il tempo t assegnato alla ricerca per eseguire le prossime movestogo mosse
// go winc i			Se deve muovere il Bianco, questo argomento imposta il tempo i che incrementera' ad ogni mossa il tempo concesso 
// go binc i			Se deve muovere il Nero, questo argomento imposta il tempo i che incrementera' ad ogni mossa il tempo concesso 
// go movetime t		Imposta il tempo per mossa a t
// go movestogo m		Imposta il numero di mosse rimanenti da eseguire nel tempo impostato
//
// Chiaramente piu' argomenti possono essere forniti
// L'unita' di misura del tempo e' il millisecondo
//
// INPUT:	line		comando ricevuto ("position...")


void ParseGo(char *line) {

	char *ptr;			// Pointer alla linea in input per il parse

	int depth;			// Valore decodificato per la profondita' (nell'esempio "depth 6")
	int movestogo;		// Valore decodificato per il numero di mosse da eseguire nel tempo rimanente concesso per la partita (time) (nell'esempio "movestogo 40")
	int movetime;		// Valore decodificato per il tempo concesso per ogni mossa (nell'esempio "movetime 1000")
	int time;			// Il tempo rimanente alla parte che ha la mossa per finire la partita
	int inc;			// L'incremento da dare ad ogni mossa al tempo concesso alla parte che deve muovere
	int timeset;		// Abilitazione gestione tempo
	int Depth;			// Profondita' di ricerca conclusiva

	// Caricamento valori di default

	depth = -1;			// = -1 significa che non e' stata imposta nessuna profondita' di ricerca
	movestogo = 30;		// Un valore ragionevole se non fosse stato settato
	movetime = -1;		// -1 significa che non e' stato imposto nessun tempo per mossa
	time = -1;			// Significa che non e' stato impostato un tempo per le mosse rimanenti
	inc = 0;			// Per default non c'e' incremento di tempo per mossa

	timeset = FALSE;	// Per default non c'e' limite di tempo (analisi infinita)

	// NOTA:	strstr parte la ricerca sempre dall'inizio della stringa, per cui si decodificheranno TUTTI gli argomenti forniti al comando "go" 

	if ((ptr = strstr(line,"infinite"))) ;								// Se ricevuto "infinite" come argomento avendo gia' settato per default info->timeset = FALSE  non bisogna fare niente

	if ((ptr = strstr(line,"winc")) && pos->side == WHITE)				// Se ricevuto "winc " come argomento e se la parte che deve muovere e' il Bianco
		inc = atoi(ptr + 5);											// estrae l'argomento del dato associato ("winc " e' 5 caratteri)

	if ((ptr = strstr(line,"binc")) && pos->side == BLACK)				// Se ricevuto "binc " come argomento e se la parte che deve muovere e' il Nero
		inc = atoi(ptr + 5);											// estrae l'argomento del dato associato ("binc " e' 5 caratteri)

	if ((ptr = strstr(line,"wtime")) && pos->side == WHITE)				// Se ricevuto "wtime " come argomento e se la parte che deve muovere e' il Bianco
		time = atoi(ptr + 6);											// estrae l'argomento del dato associato ("wtime " e' 6 caratteri)

	if ((ptr = strstr(line,"btime")) && pos->side == BLACK)				// Se ricevuto "wtime " come argomento e se la parte che deve muovere e' il Nero
		time = atoi(ptr + 6);											// estrae l'argomento del dato associato ("btime " e' 6 caratteri)

	if ((ptr = strstr(line,"movestogo")))								// Se ricevuto "movestogo " come argomento
		movestogo = atoi(ptr + 10);										// estrae l'argomento del dato associato ("movestogo " e' 10 caratteri)

	if ((ptr = strstr(line,"movetime")))								// Se ricevuto "movetime " come argomento
		movetime = atoi(ptr + 9);										// estrae l'argomento del dato associato ("movetime " e' 9 caratteri)

	if ((ptr = strstr(line,"depth")))									// Se ricevuto "depth " come argomento
		depth = atoi(ptr + 6);											// estrae l'argomento del dato associato ("depth " e' 6 caratteri)

	// Adesso settiamo il database della ricerca in base alle informazioni collezionate dal parse della linea di comando "go"

	if (movetime != -1) {												// Se e' stato impostato un tempo per mossa
	
		time = movetime;												// E' equivalente ad impostare un tempo per le mosse rimanenti della stessa entita'
		movestogo = 1;													// e dire che c'e' solo una mossa rimanente
	}

	Depth = ( depth <= 0 || depth > MAXDEPTH) ? MAXDEPTH : depth;		// Imposta profondita' voluta (se nessuna profondita' richiesta o valore fuori dai limiti imposta quella massima concessa dal dimensionamento della ram)

	if (time != -1) {													// Se e' stato impostato un tempo per un ammontare di mosse pari a movestogo
	
		timeset = TRUE;													// Bisogna attivare la gestione del tempo di mossa

		time /= movestogo;												// Ottengo cosi' il tempo per mossa richiesto (ricordiamo che se movestogo non e' indicato abbiamo un default di 30)

		time -= 50;														// Per stare nel sicuro e non sforare tolgo 50 msecs. dal tempo per mossa ricavato
	}

	DLL_Search((timeset) ? time : 0,Depth,0,&UCI_SearchStatus,UCI_BestMove,UCI_CurrentMove,&lNodes,&lScore,&lDepth);														// Finalmente parte nella ricerca									
}


//
// Funzione che implementa il protocollo UCI (Universal Chess Interface)


void Uci_Loop(void) {

	char line[INPUTBUFFER];						// Buffer di scambio dati per il protocollo
	char *str_true;								// Per ricerca sottostringa "true"
	char SearchType[128];						// Tipo ricerca
	int	 ValOpz;								// Valore opzione
	int  MB=0;									// Size HashTable
	int  Quit;									// Flag uscita

	Quit = FALSE;

	setvbuf(stdin,NULL,_IONBF,0);				// Disattiva il buffering per lo standard input
	setvbuf(stdout,NULL,_IONBF,0);				// Disattiva il buffering per lo standard output

	printf("id name "NAME"\n");				// Dichiara nome engine
	printf("id author Lelli Massimo\n");		// Dichiara autore engine
	printf("option name Hash type spin default 64 min 4 max %d\n",MAX_HASH);								// Dichiara opzione disponibile (nome Hash tipo intero range >= 4 <= 64)
	printf("option name Book type check default true\n");													// Dichiara opzione disponibile (nome Book tipo booleano default true)
	printf("option name SearchType type combo default AlphaBeta var AlphaBeta var MinMax var NegaMax\n");	// Dichiara opzione disponibile (nome SearchType tipo stringa default AlphaBeta)
	printf("option name RawEval type check default false\n");												// Dichiara opzione disponibile (nome RawEval tipo booleano default false)
	printf("option name Quiescence type check default true\n");												// Dichiara opzione disponibile (nome Quiescence tipo booleano default true)
	printf("option name HashTable type check default true\n");												// Dichiara opzione disponibile (nome HashTable tipo booleano default true)
	printf("option name NullMove type check default true\n");												// Dichiara opzione disponibile (nome NullMove tipo booleano default true)
	printf("uciok\n");
	
	ParsePosition("position startpos\n");				// Fa' come se avesse invece ricevto il comando "position startpos"

	while (TRUE) {								// Loop infinito
	
		memset(line,0,sizeof(line));				// Azzera il buffer di scambio

		fflush(stdout);								// Propedeutico anche se non si usa il buffering

		if (fgets(line,INPUTBUFFER,stdin)) {		// Ricava eventuale comando da GUI
	
			// C'e' un comando

			if (line[0] == '\n')
				continue;							// Se riga vuota aspetta altri comandi

			if (!strncmp(line,"isready",7)) {		// Interrogazione della GUI per vedere se l'engine e' pronta
		
				printf("readyok\n");				// Risposta di okay
				continue;							// Va' ad aspettare altri comandi
			}

			if (!strncmp(line,"position",8))		// Se comando "position"
				ParsePosition(line);				// Gestione comando "position"

			if (!strncmp(line,"ucinewgame",10))	{			// Se comando "ucinewgame"
				ParsePosition("position startpos\n");		// Fa' come se avesse invece ricevto il comando "position startpos" che e' equivalente
				continue;
			}

			if (!strncmp(line,"go",2))				// Se comando "go"
				ParseGo(line);						// Gestione comando "go"

			if (!strncmp(line,"quit",4))			// Se comando "quit"
				Quit = TRUE;						// Prenota l'uscita dal programma

			if (!strncmp(line,"uci",3)) {			// Se comando "uci" (richiesta identificazione)
			
				printf("id name "NAME"\n");			// Si identifica...
				printf("id author Lelli Massimo\n");
				printf("uciok\n");					
			}

			if (!strncmp(line,"setoption name Hash value ",26)) {		// Test comando "set option name Hash value " di lunghezza 26 caratteri)
#if defined(_MSC_VER) || defined(__MINGW32__)
//				sscanf_s(line,"%*s %*s %*s %*s %d",&MB);	// Ricava il numero di Megabytes da impostare (%*s fa' saltare la stringa quindi salta 4 stringhe (setoption,name,Hash,value) e carica il quinto campo intero nella variabile MB)
#else
				sscanf(line,"%*s %*s %*s %*s %d",&MB);		// Ricava il numero di Megabytes da impostare (%*s fa' saltare la stringa quindi salta 4 stringhe (setoption,name,Hash,value) e carica il quinto campo intero nella variabile MB)
#endif
				SetHashSize(MB);										// Setta la memoria della hashtable

				printf("Set Hash to %d MB\n",MB);						// Stampa nr. megabytes decodificati da comando
			}

			if (!strncmp(line,"setoption name Book value ",26)) {		// Test comando per attivare/disattivare la libreria delle aperture
			
				str_true = strstr(line,"true");							// Cerca la sottostringa "true"

				ValOpz = (str_true == NULL) ? FALSE : TRUE;

				SetOpt(BOOK,ValOpz);									// Setta in maniera opportuna il permesso a utilizzare la libreria

				if (ValOpz)
					printf("Set Book to true\n");
				else
					printf("Set Book to false\n");
			}

			if (!strncmp(line,"setoption name SearchType value ",32)) {		// Test comando "set option name SearchType value " di lunghezza 32 caratteri)
#if defined(_MSC_VER) || defined(__MINGW32__)
				sscanf_s(line,"%*s %*s %*s %*s %s",SearchType,sizeof(SearchType));	// Tipo ricerca (%*s fa' saltare la stringa quindi salta 4 stringhe (setoption,name,Hash,value)
#else
				sscanf(line,"%*s %*s %*s %*s %s",SearchType);		// Tipo ricerca (%*s fa' saltare la stringa quindi salta 4 stringhe (setoption,name,Hash,value)
#endif
				if (!strcmp(SearchType,STR_ALPHABETA))						// Decodifica tipi ricerca
					ValOpz = ALPHABETA;

				if (!strcmp(SearchType,STR_MINMAX))
					ValOpz = MINMAX;

				if (!strcmp(SearchType,STR_NEGAMAX))
					ValOpz = NEGAMAX;

				SetOpt(SEARCHTYPE,ValOpz);									// Setta in maniera opportuna il tipo ricerca

				printf("Set SearchType to %s\n",SearchType);				// Stampa tipo di ricerca
			}

			if (!strncmp(line,"setoption name RawEval value ",29)) {		// Test comando per attivare/disattivare la valutazione grezza
			
				str_true = strstr(line,"true");								// Cerca la sottostringa "true"

				ValOpz = (str_true == NULL) ? FALSE : TRUE;					

				SetOpt(RAWEVAL,ValOpz);										// Setta in maniera opportuna il tipo di valutazione

				if (ValOpz)
					printf("Set RawEval to true\n");
				else
					printf("Set RawEval to false\n");
			}

			if (!strncmp(line,"setoption name Quiescence value ",32)) {			// Test comando per attivare/disattivare la ricerca per quiescenza
			
				str_true = strstr(line,"true");									// Cerca la sottostringa "true"

				ValOpz = (str_true == NULL) ? FALSE : TRUE;		

				SetOpt(QUIESCENCE,ValOpz);										// Setta in maniera opportuna se la ricerca ha la gestione della quiescenza

				if (ValOpz)
					printf("Set Quiescence to true\n");
				else
					printf("Set Quiescence to false\n");
			}

			if (!strncmp(line,"setoption name HashTable value ",31)) {		// Test comando per attivare/disattivare l'utilizzo della hash table
			
				str_true = strstr(line,"true");								// Cerca la sottostringa "true"

				ValOpz = (str_true == NULL) ? FALSE : TRUE;				

				SetOpt(HASH,ValOpz);										// Setta in maniera opportuna se la ricerca ha la gestione della hash table

				if (ValOpz)
					printf("Set HashTable to true\n");
				else
					printf("Set HasTable to false\n");
			}

			if (!strncmp(line,"setoption name NullMove value ",30)) {		// Test comando per attivare/disattivare l'utilizzo della mossa nulla
			
				str_true = strstr(line,"true");								// Cerca la sottostringa "true"

				ValOpz = (str_true == NULL) ? FALSE : TRUE;				

				SetOpt(NULLMOVE,ValOpz);									// Setta in maniera opportuna se la ricerca ha la gestione della mossa nulla

				if (ValOpz)
					printf("Set NullMove to true\n");
				else
					printf("Set NullMove to false\n");
			}
		}
	
		if (Quit)								// Se richiesta uscita
			break;								// esce dal loop infinito (e quindi anche dal programma)
	}
}
