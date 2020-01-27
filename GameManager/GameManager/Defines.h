#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <stdio.h>
#include <setjmp.h>
#include <assert.h>

#include "GameManager.h"

#define MAXORDERS	32

#define	MAXP	0
#define MINP	1



#define ISMATE (WIN_SCORE - MAXDEPTH)			// Il punteggio di matto e' sempre > ISMATE o < -ISMATE (essendo WIN_SCORE - pos->ply o LOSS_SCORE + pos->ply ed essendo pos->ply < MAXDEPTH) 

typedef int		(*PCHECKDRAW)(void);
typedef int		(*PCHECKLOSS)(void);
typedef int		(*PCHECKWIN)(void);
typedef int		(*PEVAL)(int RawEval);
typedef char*	(*PGENMOVEALLOWED)(MList *movelist,int quiesc);
typedef int		(*PMAKEMOVE)(CompMove *move);
typedef void	(*PTAKEBACK)(void);
typedef int		(*PEXTRADEPTH)(void);
typedef int		(*PISPOSOK)(void);
typedef U64		(*PGETHASHKEY)(void);
typedef void	(*PMAKENULLMOVE)(void);
typedef void	(*PTAKENULLMOVE)(void);
typedef void	(*PINITSEARCH)(void);
typedef int		(*PISDRAW)(void);
typedef int		(*PGETPLY)(void);
typedef int		(*PGETSIDE)(void);
typedef int		(*PCANDONULL)(void);
typedef CompMove  (*PGETBOOKMOVE)(void);
typedef	void	(*PSETHISTORY)(CompMove move,int depth);
typedef	void	(*PSETKILLERS)(CompMove *move);
typedef char*	(*PPRINTMOVE)(CompMove *move);
typedef int		(*PINITDLL)(void);
typedef int		(*PGETPVSCORE)(void);


//
// Valori assunti dal campo flags nel record HASHENTRY della tabella hash
//

enum { HFNONE, HFALPHA, HFBETA, HFEXACT };

//
// Struttura che definisce un record nella hashtable
// I ptimi 2 campi:  il primo e' il valore della chiave della posizione corrispondente
//					 il secondo e' la mossa eseguita in quella posizione
//
// Quindi il singolo entry della hashtable memorizza in pratica un nodo nell'albero delle mosse
// in quanto memorizza la posizione e la mossa eseguita nella posizione.
// La posizione e' un nodo dell'albero e la mossa mi porta a una nuova posizione cioe'
// collega il nodo attuale al successivo
//

typedef struct {

	U64 posKey;				// Hash key completa su 64 bit
	CompMove move;			// Mossa codificata
	int score;				// Punteggio della mossa
	int depth;				// Profondita' della mossa nella ricerca
	int flags;				// Tipo di entry (HFALPHA,HFBETA,HFEXACT)

} S_HASHENTRY;


//
// Questa struttura rappresenta la hashtable che
// altro non e' che un vettore di elementi del tipo S_HASHENTRY
//
// Nella struttura e' indicato il numero di elementi del vettore
// ma esso non e' esplicitamente allocato;
// viene semplicente definita la testa del vettore che
// in fase di inizializzazione verra' fatta puntare
// ad un area di memoria esterna allocata dinamicamente.
//

typedef struct {

	S_HASHENTRY *pTable;		// Pointer alla ram allocata dinamicamente
	int numEntries;				// Numero massimo di record memorizzabili
	int newWrite;				// Counter che si incrementa ogni volta che viene memorizzata una nuova entry
	int overWrite;				// Counter che si incrementa ogni volta che una entry viene sovrascritta
	int hit;					// Counter che si incrementa ogni volta che la posizione viene trovata (con la profondita' opportuna) nella hashtable
	int cut;					// Counter che si incrementa ogni volta che la posizione viene trovata (con la profondita' opportuna) nella hashtable E l'informazione permette di assumere
								// come punteggio della posizione quella memorizzata nella hash, permettendo di evitare di continuare la ricerca

} S_HASHTABLE;

//
// Varie informazioni sulla ricerca corrente
//

typedef struct {

	int starttime;						// Istante inizio ricerca
	int stoptime;						// Istante da controllare per interrompere la ricerca
	int depth;							// Profondita' massima richiesta nella ricerca (se vale MAXDEPTH significa che in pratica il fine ricerca dipendera' solo dal tempo)
	int depthset;
	int timeset;						// Se = TRUE e' attivo il controllo del tempo nella ricerca (se disattivo la fine ricerca dipendera' solo dal depth massimo concesso)
	int movestogo;						// Numero di mosse da eseguire nel tempo concesso dal quale si ricava il tempo per la ricerca (stoptime - starttime)
	int infinite;

	U64 nodes;							// Numero di nodi esplorati nella ricerca

	int quit;							// Se posto a 1 si abortisce la eventuale ricerca e comunque si esce dal programma
	int stopped;						// Se posto a TRUE, la ricerca si deve interrompere scartando l'iterative deepening attuale e tenendo come buoni i risultati ottenuti nella ricerca
										// effettuata alla profondita' precedente

	int fh;								// Contatore fail-high (beta cutoff)
	int fhf;							// Contatore fail-high-first (beta cutoff al primo ramo esaminato)
	int nullCut;						// Contatore beta cutoff in ricerca con mossa nulla

	// Info per la GUI

	Search_Status *status;				// Stato della GUI
	LPSTR bMove;						// Mossa migliore
	LPSTR currentMove;					// Mossa in ricerca
	long *plNodes;						// Nr. nodi visitati
	long *plScore;						// Punteggio corrente
	long *plDepth;						// Profondita' corrente

} S_SEARCHINFO;

//
// Questa struttura definisce le varie opzione per il motore
//

typedef struct {

	int Book;						// = TRUE se si puo' utilizzare la libreria delle aperture
	int SearchType;						// Tipo ricerca (0 = AlphaBeta   1 = MinMax   2 = NegaMax)
	int RawEval;						// = TRUE valutazione grezza (default = FALSE)
	int Quiescence;						// Ricerca con quiescenza (default = TRUE)
	int Hash;							// Gestione hash attiva (default = TRUE)
	int NullMove;						// Gestione Null Move attiva (default = TRUE)

} S_OPTIONS;


typedef struct
{
	CompMove Pv[MAXDEPTH];
	int nel;
} S_PVARRAY;




void InitHashTable(S_HASHTABLE *table,int MB);
void ClearHashTable(S_HASHTABLE *table);
int ProbeHashEntry(U64 hashKey,CompMove *move, int *score,int alpha,int beta,int depth);
void StoreHashEntry(U64 hashKey,CompMove move,int score,int flags,int depth);
int GetTimeMs();
int ProbePvMove(U64 hashKey,CompMove *move);
int GetPvLine(int depth);

void CheckUp(void);
void PickNextMove(int moveNum,MList *list);
int Min1(int depth);
int Max1(int depth);
int NegaMax(int depth);
int Quiescence(int alpha,int beta);
int AlphaBeta(int alpha,int beta,int depth,int DoNull);
void SearchPosition(void);
void GenStrMoves(MList *movelist);
DLL_Result DoMove(char *move);
int MoveExists(CompMove move);
int CompareMoves(CompMove *move1,CompMove *move2);
int InputWaiting();

extern S_HASHTABLE HashTable;
extern S_SEARCHINFO info[];
extern S_OPTIONS EngineOptions[];
extern S_PVARRAY PvArray;
extern PCHECKDRAW pCheckDraw;
extern PCHECKLOSS pCheckLoss;
extern PCHECKWIN pCheckWin;
extern PEVAL pEval;
extern PTAKEBACK pTakeBack;
extern PEXTRADEPTH pExtraDepth;
extern PISPOSOK pIsPosOk;
extern PGENMOVEALLOWED pGenMoveAllowed;
extern PGETHASHKEY pGetHashKey;
extern PMAKEMOVE pMakeMove;
extern PMAKENULLMOVE pMakeNullMove;
extern PTAKENULLMOVE pTakeNullMove;
extern PINITSEARCH pInitSearch;
extern PISDRAW pIsDraw;
extern PGETPLY pGetPly;
extern PGETSIDE pGetSide;
extern PCANDONULL pCanDoNull;
extern PGETBOOKMOVE pGetBookMove;
extern PSETHISTORY pSetHistory;
extern PSETKILLERS pSetKillers;
extern PPRINTMOVE pPrintMove;
extern PINITDLL pInitDll;
extern PGETPVSCORE pGetPvScore;
extern char order[][MAXORDERS][MAXNAMES+1];
extern int	nvar;
extern int nplayers_to_move[];
extern int UciMode;
