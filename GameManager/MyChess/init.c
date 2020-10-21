#include "defs.h"

// include NNUE probe library wrapper header
#include "nnue_eval.h"

//
// Questa macro serve per generare un intero casuale a 64 bit
// Viene utilizzata la funzione rand() che genera un intero casuale a 15 bit
// Viene chiamata 4 volte shiftando per 3 volte i 15 bit verso sinistra e facendo l'or
// In tal modo si ottiene un numero random a 15*4 = 60 bit
// Si chiama di nuovo rand() tenendo solo i 4 bit piu' bassi; poi
// tutto viene shiftato di 60 verso sinistra e viene fatto di nuovo la somma;
// Alla fine ho un intero di 15*4 + 4 = 64 bit
//

#define RAND_64	(	(U64)rand() | \
					( (U64)rand() << 15 ) | \
					( (U64)rand() << 30 ) | \
					( (U64)rand() << 45 ) | \
					( ( (U64)rand() & 0x0f ) << 60 )	)

// C'e' l'esigenza di passare agevolmente da una casella nel vettore scacchiera estesa
// alla corrispondente casella nella bitboard e viceversa.
// Avremo quindi bisogno di fare un conversione,in entrambe le direzioni, fra l'indice
// della casella nella scacchiera estesa e l'indice della casella nella bitboard.
//
// Per convertire l'indice della scacchiera estesa (0-119) in quello della bitboard (0-63)
// utilizzeremo il vettore Sq120ToSq64
// Per convertire l'indice della della bitboard (0-63) in quello della scacchiera estesa (0-119)
// utilizzeremo il vettore Sq64ToSq120.
// Questi 2 vettori dovranno essere inizializzati con i valori opportuni per la conversione.

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

//
// Maschere per settare e resettare un bit in una bitboard
//

U64 SetMask[64];
U64	ClearMask[64];

//
// Valori a 64 bit casuali per calcolare l'hashkey di una posizione
//

U64 PieceKeys[13][BRD_SQ_NUM];		// Per ogni tipo di pezzo e per ogni casella della scacchiera estesa
U64 SideKey;						// Contribuisce all'hashkey quando muove il bianco
U64 CastleKeys[16];					// UN valore per ogni possibile codice di permesso dell'arrocco (4 bit : 0-15)

//
// Tabelle per ottenere a partire dalla casella della scacchiera estesa
// il numero di colonna e traversa nella scacchiera reale
//

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

//
// Tabella di 64 bitboard (una per ogni casella della scacchiera reale)
// Sono settati a 1 quei bit relativi alle possibili posizioni di pedoni del
// Nero che renderebbero il pedone del Bianco (presente nella casella di riferimento)
// un pedone NON passato
//

U64 WhitePassedMask[64];

//
// Tabella di 64 bitboard (una per ogni casella della scacchiera reale)
// Sono settati a 1 quei bit relativi alle possibili posizioni di pedoni del
// Bianco che renderebbero il pedone del Nero (presente nella casella di riferimento)
// un pedone NON passato
//

U64 BlackPassedMask[64];

//
// Tabella di 64 bitboard (una per ogni casella della scacchiera reale)
// Sono settati a 1 quei bit relativi alle possibili posizioni di pedoni "amici"
// che renderebbero il pedone (presente nella casella di riferimento)
// un pedone NON isolato
//

U64 IsolatedMask[64];

// NOTA:	Le 3 tabelle precedenti sono generate anche per posizioni di bit che non sono lecite:
//			cioe' ad es. pedoni sulla prima o ottava traversa. Queste tabelle vengono comunque generate
//			ma non saranno mai utilizzate!

//
// 8 bitboard (una relativa ad ogni colonna) che tutti bit a 0 fuorche' quelli della colonna suddetta 
//

U64 FileBBMask[8];

//
// 8 bitboard (una relativa ad ogni traversa) che tutti bit a 0 fuorche' quelli della traversa suddetta 
//

U64 RankBBMask[8];



//
// Questa funzione inizializza le tabelle WhitePassedMask,BlackPassedMask e IsolatedMask
// Inizializza anche le 2 tabelle FileBBNask e RankBBMask
//
// NOTA:	Va chiamata in fase di inizializzazione DOPO la InitFilesRanksBrd()
//			in quanto utilizza i vettori FilesBrd[] e RanksBr[] che devono quindi
//			gia' essere stati inizializzati

void InitEvalMasks(void) {

	int sq;				// Casella nella scacchiera reale (0 - 63)
	int tsq;			// Appoggio per sq
	int file;			// Indice file
	int rank;			// Indice rank
	U64 mask;			// Maschera bits

	// Per prima cosa genera le 2 tabelle FileBBMask e RankBBMask

	for (file = FILE_A; file <= FILE_H; file++)			// Azzera vettore FileBBMask
		FileBBMask[file] = 0ULL;

		for (rank = RANK_1; rank <= RANK_8; rank++)		// Azzera vettore RankBBMask
		RankBBMask[rank] = 0ULL;

		mask = 1ULL;									// Inizializzo maschera

	for (rank = RANK_1; rank <= RANK_8; rank++)
		for (file = FILE_A; file <= FILE_H; file++) {	// Loop dal bit 0 al 63 delle bitboard

			FileBBMask[file] |= mask;
			RankBBMask[rank] |= mask;

			mask <<= 1;									// Shift left di 1 (passa alla prossima casella)
		}

#if 0
	for (rank = RANK_8; rank >= RANK_1; rank--)
		PrintBitBoard(RankBBMask[rank]);

	for (file = FILE_A; file <= FILE_H; file++)
		PrintBitBoard(FileBBMask[file]);
#endif

	// Ora genera le tabelle WhitePassedMask BlackPassedMask e IsolatedMask

	for (sq = 0; sq < 64; sq++) {				// Come prima cosa azzera i 3 vettori di 64 elementi da inizializzare
	
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
		IsolatedMask[sq] = 0ULL;
	}

	for (sq = 0; sq < 64; sq++) {

		if (FilesBrd[SQ120(sq)] > FILE_A) {								// Solo dopo la prima colonna

			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];	// Setta il bit a sinistra del pedone accedendo alla tabella FileBBMask con un numero di colonna + 1

			tsq = sq - 1;									// Si porta sulla colonna a sinistra

			while ( (tsq += 8) < 64)
				WhitePassedMask[sq] |= (1ULL << tsq);		// Setta tutti i bit davanti al pedone bianco e nella colonna a sinistra (traversa + 1 e colonna - 1 (corrisponde a casella + 8 - 1)

			tsq = sq - 1;									// Si porta sulla colonna a sinistra

			while ( (tsq -= 8) >= 0)
				BlackPassedMask[sq] |= (1ULL << tsq);		// Setta tutti i bit davanti al pedone nero e nella colonna a sinistra (traversa - 1 e colonna - 1 (corrisponde a casella - 8 - 1)
		}

		tsq = sq;

		while ( (tsq += 8) < 64)
			WhitePassedMask[sq] |= (1ULL << tsq);		// Setta tutti i bit davanti al pedone bianco (traversa + 1) (corrisponde a casella + 8)

		tsq = sq;

		while ( (tsq -= 8) >= 0)
			BlackPassedMask[sq] |= (1ULL << tsq);		// Setta tutti i bit davanti al pedone nero (traversa - 1) (corrisponde a casella - 8)

		if (FilesBrd[SQ120(sq)] < FILE_H) {				// Solo prima dell'ultima colonna

			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];	// Setta il bit a destra del pedone accedendo alla tabella FileBBMask con un numero di colonna - 1

			tsq = sq + 1;									// Si porta sulla colonna a destra

			while ( (tsq += 8) < 64)
				WhitePassedMask[sq] |= (1ULL << tsq);		// Setta tutti i bit davanti al pedone bianco e nella colonna a destra (traversa + 1 e colonna + 1) (corrisponde a casella + 8 + 1)

			tsq = sq + 1;									// Si porta sulla colonna a destra

			while ( (tsq -= 8) >= 0)
				BlackPassedMask[sq] |= (1ULL << tsq);		// Setta tutti i bit davanti al pedone nero e nella colonna a destra (traversa - 1 e colonna + 1) (corrisponde a casella - 8 + 1)
		}
	}

#if 0
	for (sq = 0; sq < 64; sq++) {
		PrintBitBoard(WhitePassedMask[sq]);
		PrintBitBoard(BlackPassedMask[sq]);
		PrintBitBoard(IsolatedMask[sq]);
		getchar();
	}
#endif
}

//
// Funzione per inizializzare le tabelle FilesBrd e RanksBrd
//

void InitFilesRanksBrd(void) {

	int index;				// Indice nella scacchiera estesa
	int file;				// colonna corrente
	int rank;				// traversa corrente
	int sq;					// Numero casella in scacchiera estesa


	for (index = 0; index < BRD_SQ_NUM; index++) {			// Preinizializza le due tabelle con valori OFFBOARD che rimarrano appunto per le caselle della scaccchiera estesa non appartenenti a quella reale
	
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	}

	for (rank = RANK_1; rank <= RANK_8; rank++)				// Loop 8 traverse
		for (file = FILE_A; file <= FILE_H; file++) {		// Loop 8 colonne
		
			sq = FR2SQ(file,rank);							// Ricava numero casella in scacchiera estesa

			FilesBrd[sq] = file;							// Carica numero colonna
			RanksBrd[sq] = rank;							// Carica numero traversa
		}

#if 0
		DoLog("FilesBrd\n");
		for (index=0;index<BRD_SQ_NUM;index++) {
			if (index%10==0 && index!=0) DoLog("\n");
			DoLog("%4d",FilesBrd[index]);
		}
		DoLog("\n\n");
		DoLog("RanksBrd\n");
		for (index=0;index<BRD_SQ_NUM;index++) {
			if (index%10==0 && index!=0) DoLog("\n");
			DoLog("%4d",RanksBrd[index]);
		}
		DoLog("\n\n");
		getchar();
#endif
}

//
// Init delle tabelle per il calcolo dell'hash key di una posizione
//

void InitHashKeys(void) {

	int index;
	int index2;

	for (index = 0; index < 13; index++)
		for (index2 = 0; index2 < BRD_SQ_NUM; index2++)
			PieceKeys[index][index2] = RAND_64;

	SideKey = RAND_64;

	for (index = 0; index < 16; index++)
		CastleKeys[index] = RAND_64;
}


//
//
// Inizializzazione delle tabelle SetMask[] e ClearMask[]
//
//

void InitBitMask(void) {

	int index;		// indice nei vettori

	// Prima azzera le matrici

	for (index = 0; index < 64; index++) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for (index = 0; index < 64; index++) {
		SetMask[index] = (1ULL << index);			// Maschera per l'OR
		ClearMask[index] = ~SetMask[index];			// Maschera per l'AND
	}
}

//
//
// Inizializzazione dei vettori Sq120ToSq64 e Sq64ToSq120
//
//

void InitConversionTables(void) {

	int index;		// indice ai vettori
	int file;		// colonna corrente
	int rank;		// riga corrente
	int sq;			// casella in scacchiera estesa

	for (index = 0; index < BRD_SQ_NUM; index++)
		Sq120ToSq64[index] = 64;			// Precarico un valore di indice non valido in modo che rimanga
											// tale valore per le locazioni della scacchiera estesa non esistenti in quella reale
											// Non serve farlo anche per il vettore Sq64To120 poiche' a una casella della scacchiera
											// reale esistera' sempre una corrispondente nella scacchiera estesa.
											// Non e' vero invece il viceversa.

	index=0;								// init indice per scacchiera reale 

	for (rank = RANK_1; rank <= RANK_8; rank++)				// loop righe
		for (file = FILE_A; file <= FILE_H; file++) {		// loop colonne
			sq = FR2SQ(file,rank);
			Sq64ToSq120[index] = sq;
			Sq120ToSq64[sq] = index;
			index++;
		}

}

// Init una volta per tutte di vari dati

void AllInit(void) {
int result;
	
	InitConversionTables();					// Init tabelle conversione tra scacchiera reale ed estesa
	InitBitMask();							// Init tabelle per settare e resettare un bit in una bitboard
	InitHashKeys();							// Init tabelle per il calcolo dell'hash key di una posizione
	InitFilesRanksBrd();					// Genera tabelle FilesBrd e RanksBrd
	InitEvalMasks();						// Init maschere per valutazione strutture dei pedoni
	InitMvvLva();							// Init matrice punteggi mosse di cattura
	IniPolyBook();							// Init libreria delle aperture
	
        // init Stockfish NNUE
	if (GetOpt(USENNUE))
	{
		DoLog("Loading NNUE: net.nnue");
        	result=init_nnue("net.nnue");
		if (result==0)
			DoLog("NNUE file not found!");
		else
			DoLog("NNUE loaded!");

	}
}
