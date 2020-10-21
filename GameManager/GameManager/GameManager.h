#ifdef __cplusplus
extern "C" {
#endif
	
#include "Includes.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#define LPCWSTR const char *
#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#define INFINITE -1

typedef struct {
	char cFileName[256+1];
} WIN32_FIND_DATAA;

// Le 2 costanti booleane falso=0 e vero=1

enum { FALSE, TRUE };

#endif

typedef	unsigned long long	U64;			// Tipo intero senza segno a 64 bit

// Tipi di ricerca

#define	STR_ALPHABETA	"AlphaBeta"
#define	STR_MINMAX	"MinMax"
#define STR_NEGAMAX	"NegaMax"


#define	FILES		0
#define	RANKS		1

#define MAXDEPTH	64

#define	MAXNAMES	64
#define	MAXUDIMS	64

#define	MAXMOVES	256
#define	MAXLMOVE	64
#define	MAXBMOVES	2

#define	MAXEMV		4

#define	CAPTURE		0x01

#define PRSPLAY		0x01


#define	NOMOVE		0				// Valore di mossa codificata errata (una mossa corretta codificata non potra' mai valere 0)

// Tipi ricerca

#define	ALPHABETA	0
#define	MINMAX		1
#define	NEGAMAX		2


#define MAX_HASH	1024			// Nr. massimo di Megabytes allocabili per la hash table


typedef struct
{
	int file;
	int rank;
	int piece;
	int flags;
} Entity;

typedef struct
{
	Entity from;
	Entity to[MAXEMV];
	int n_to_moves;
} BasicMove;

typedef struct
{
	BasicMove bmove[MAXBMOVES];
	char str_moves[MAXLMOVE];
	int flags;
	int nbmoves;
	int score;			// Punteggio mossa
} CompMove;

typedef struct
{
	CompMove m[MAXMOVES];
	int nmoves;
} MList;


// Codici delle opzioni

enum { BOOK,SEARCHTYPE,RAWEVAL,USENNUE,QUIESCENCE,HASH,NULLMOVE };

#ifdef GAMEMANAGER_EXPORTS
#define	OURLIB_API	__declspec(dllexport)
#elif	defined(GAMEMANAGER_IMPORTS)
#define	OURLIB_API	__declspec(dllimport)
#else
#define	OURLIB_API
#endif


OURLIB_API	void GetDims(int ndim,int *dim);
OURLIB_API	void DoLog(char *str, ...);
OURLIB_API	void GenVers(char *version);
OURLIB_API	void SetOpt(int Opz,int Value);
OURLIB_API	int  GetOpt(int Opz);
OURLIB_API	void SetHashSize(int HashSize);
OURLIB_API	void SetUci(void);
OURLIB_API	int  GetUci(void);
OURLIB_API	DLL_Result FAR PASCAL DLL_Search(long lSearchTime,long lDepthLimit,long lVariety,Search_Status *pSearchStatus,LPSTR bestMove,LPSTR currentMove,long *plNodes,long *plScore,long *plDepth);

#undef	OURLIB_API

#ifdef GAME_EXPORTS
#define	OURLIB_API	__declspec(dllexport)
#elif	defined(GAME_IMPORTS)
#define	OURLIB_API	__declspec(dllimport)
#else
#define	OURLIB_API
#endif

OURLIB_API	int GenMoveAllowed(MList *movelist,int quiesc);
OURLIB_API	void PrintBoard(void);
OURLIB_API	void TakeBack(void);
OURLIB_API	void EmptySquare(int rank,int file);
OURLIB_API	void SetSquare(int rank,int file,int player,int p);
OURLIB_API	int MakeMove(CompMove *movelist);
OURLIB_API	void NewGame(void);
OURLIB_API	int CheckDraw(void);
OURLIB_API	int CheckLoss(void);
OURLIB_API	int CheckWin(void);
OURLIB_API	int Eval(int RawEval);
OURLIB_API	void CleanUp(void);
OURLIB_API	int IsPosOk(void);
OURLIB_API	int ExtraDepth(void);
OURLIB_API	U64 GetHashKey(void);
OURLIB_API	void InitSearch(void);
OURLIB_API	int IsDraw(void);
OURLIB_API	int GetPly(void);
OURLIB_API	int GetSide(void);
OURLIB_API	int CanDoNull(void);
OURLIB_API	CompMove GetBookMove(void);
OURLIB_API	void SetHistory(CompMove move,int depth);
OURLIB_API	void SetKillers(CompMove *move);
OURLIB_API	char *PrintMove(CompMove *move);
OURLIB_API	void MakeNullMove(void);
OURLIB_API	void TakeNullMove(void);
OURLIB_API	int InitDll(void);
OURLIB_API	int GetPvScore(void);
OURLIB_API	void Uci_Loop(void);

OURLIB_API	void PrintBoardG(int prf);
	
	
	
typedef int             (*PCHECKDRAW)(void);
typedef int             (*PCHECKLOSS)(void);
typedef int             (*PCHECKWIN)(void);
typedef int             (*PEVAL)(int RawEval);
typedef char*   (*PGENMOVEALLOWED)(MList *movelist,int quiesc);
typedef int             (*PMAKEMOVE)(CompMove *move);
typedef void    (*PTAKEBACK)(void);
typedef int             (*PISPOSOK)(void);
typedef U64             (*PGETHASHKEY)(void);
typedef void    (*PINITSEARCH)(void);
typedef int             (*PINITDLL)(void);
typedef int             (*PGETPLY)(void);
typedef int             (*PGETSIDE)(void);	
	
typedef int             (*PEXTRADEPTH)(void);
typedef void    (*PMAKENULLMOVE)(void);
typedef void    (*PTAKENULLMOVE)(void);
typedef int             (*PISDRAW)(void);
typedef int             (*PCANDONULL)(void);
typedef CompMove  (*PGETBOOKMOVE)(void);
typedef void    (*PSETHISTORY)(CompMove move,int depth);
typedef void    (*PSETKILLERS)(CompMove *move);
typedef char*   (*PPRINTMOVE)(CompMove *move);
typedef int             (*PGETPVSCORE)(void);

#ifdef __cplusplus
}
#endif
