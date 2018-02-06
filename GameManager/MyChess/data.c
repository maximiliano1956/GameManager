#include "defs.h"

// Vettori di caratteri utilizzati per stampare una posizione

char *PceChar = ".PNBRQKpnbrqk";		// Indicizzato dal codice del pezzo (0-12)
char *SideChar = "wb-";					// Indicizzato dal side to move (2 = BOTH)
char *RankChar = "12345678";			// Indicizzato dal numeor di traversa
char *FileChar = "abcdefgh";			// Indicizzato dal numero di colonna

//
// Vettori indicizzati con il codice del pezzo 
//

int PieceBig[] = { FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE };				// TRUE se il pezzo corrispondente e' una figura (cio' e' vero in pratica se non e' il codice di empy o pedone)
int PieceMaj[] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE };			// TRUE se il pezzo corrispondente e' una figura maggiore (cioe' torre,re o regina)
int PieceMin[] = { FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE };			// TRUE se il pezzo corrispondente e' una figura minore (coie' cavallo o alfiere)
int PieceVal[] = { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000};							// Valore di ogni pezzo in centesimi di pedone (il valore del RE non verra' usato)
int PieceCol[] = { BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK };		// Colore corrispondente a ogni tipo di pezzo (per il codice EMPTY in pratica non ha senso)

int PiecePawn[] = { FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };		// TRUE se il pezzo corrispondente e' un Pedone
int PieceKnight[] = { FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE };	// TRUE se il pezzo corrispondente e' un Cavallo
int PieceKing[] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE };		// TRUE se il pezzo corrispondente e' un Re
int PieceRookQueen[] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE };	// TRUE se il pezzo cosrripondente e' una Torre o una Regina
int PieceBishopQueen[] = { FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE };	// TRUE se il pezzo corrispondente e' un Alfiere o una Regina

int PieceSlides[] = { FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE };		// TRUE se il pezzo e' in grado di minacciare a distanza (Alfiere,Torre,Regina)


// Questa tabella serve definisce per ogni casella nella scacchiera reale,
// quella simmetrica rispetto una linea di simmetria orizzontale che passa
// per il centro scacchera

int Mirror64[] = {
56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
};