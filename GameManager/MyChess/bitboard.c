#include "defs.h"

const int BitTable[64] = {
	63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
	51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
	26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
	58, 20, 37, 17, 36, 8
};

//
// Questa funzione restituisce l'indice del bit a 1 di peso piu' basso ,relativamente alla parola a 64 bit in input;
// Questa funzione ALTERA l'ingresso in quanto il bit suddetto viene posto a 0
// NOTA: Pop e' abbreviativo per Population
//
int PopBit(U64 *bb) {
	U64 b;
	unsigned int fold;

	b = *bb ^ (*bb - 1);
	fold = (unsigned int) ( (b & 0xffffffff) ^ (b >> 32) );
	*bb &= (*bb - 1);
	return (BitTable[(fold * 0x783a9b23) >> 26]);
}

//
// Questa funzione restituisce il numero di bit a 1 nella parola a 64 bit in ingresso
//

int CountBits(U64 b) {
	int r;

	for (r = 0; b; r++, b &= b - 1);
	return r;
};


// Funzione per stampare una bitboard

void PrintBitBoard(U64 bb) {

	int rank;		// riga
	int file;		// colonna
	int sq64;		// indice casella nella bitboard

	// Notare che la stampa parte dalla riga 8 e va' a ritroso alla riga 1 in modo da stampare
	// la board con il bianco nella parte bassa

	for (rank = RANK_8; rank>=RANK_1; rank--) {
		for (file = FILE_A; file<=FILE_H; file++) {
			sq64=SQ64(FR2SQ(file,rank));				// Ricava indice in bitboard corrispondente alla posizione file,rank
			if (bb & (1ULL << sq64))					// Fa un and con la maschera per isolare il bit nr. sq64
														// 1ULL sta per 1 unsigned long long cioe' la costante 1 a 64 bit
				DoLog("X");
			else
				DoLog("-");
		}
		DoLog("\n");
	}
	DoLog("\n");
}