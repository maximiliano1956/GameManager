#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <unistd.h>

#include <Includes.h>

#define WHITE	0
#define BLACK	1

#define	MAN	0
#define AUTO	1

typedef void (*PPRINTBOARDG)(int prf);

typedef DLL_Result FAR PASCAL (*PDLLMAKEAMOVE)(LPCSTR move);
typedef DLL_Result FAR PASCAL (*PDLLGENERATEMOVES)(LPCSTR moveBuffer);
typedef DLL_Result FAR PASCAL (*PDLLSTARTNEWGAME)(LPCSTR variant);
typedef DLL_Result FAR PASCAL (*PDLLISGAMEOVER)(long *lResult, LPSTR zcomment);
typedef DLL_Result FAR PASCAL (*PDLLSEARCH)(long lSearchTime,long lDepthLimit,long lVariety,Search_Status *pSearchStatus,LPSTR bestMove,LPSTR currentMove,long *plNodes,long *plScore,long *plDepth);
typedef DLL_Result FAR PASCAL (*PDLLCLEANUP)();

PPRINTBOARDG pPrintBoardG;
PDLLMAKEAMOVE pDllMakeAMove;
PDLLGENERATEMOVES pDllGenerateMoves;
PDLLSTARTNEWGAME pDllStartNewGame;
PDLLISGAMEOVER pDllIsGameOver;
PDLLSEARCH pDllSearch;

char listMoves[256][32];
int nmoves;

char currentMove[256];

pthread_t thread1;
int  iret1;
int stop_t;

HMODULE hLib_man=NULL;
HMODULE hLib_chess=NULL;

void MoveAllowed(void)
{
	char moveBuffer[32];
	int nm;

        (*pDllGenerateMoves)(NULL);
        nmoves=0;
        do
                (*pDllGenerateMoves)(moveBuffer);
        while (strlen(moveBuffer)!=0 && (strcpy(listMoves[nmoves++],moveBuffer) || 1));
}

void *monitor(void *dummy)
{
	while (!stop_t)
	{
		printf("%s",currentMove);
		sleep(1);
	}
}


int main(char argc, char argv[]) {
	Search_Status stato;
	char bestMove[256];
	long plNodes;
	long plScore;
	long plDepth;
	long lResult;
	char zcomment[256];
	int side;
	int ngames;
	int ng;
	int nmossa;
	int player[2];
	time_t t;
	long msecs;
	long maxdepth;

	if ((hLib_man=LoadLibrary("GameManager"))==NULL)
	{
		printf("Can't load GameManager.dll\n");
		return(1);
	}

	if ((hLib_chess=LoadLibrary("MyChess"))==NULL)
	{
		printf("Can't load MyChess.dll\n");
		FreeLibrary(hLib_man);
		return(1);
	}

	pDllMakeAMove=(PDLLMAKEAMOVE)GetProcAddress(hLib_man,"DLL_MakeAMove");
	pDllGenerateMoves=(PDLLGENERATEMOVES)GetProcAddress(hLib_man,"DLL_GenerateMoves");
	pDllStartNewGame=(PDLLSTARTNEWGAME)GetProcAddress(hLib_man,"DLL_StartNewGame");
	pDllIsGameOver=(PDLLISGAMEOVER)GetProcAddress(hLib_man,"DLL_IsGameOver");
	pDllSearch=(PDLLSEARCH)GetProcAddress(hLib_man,"DLL_Search");

	pPrintBoardG=(PPRINTBOARDG)GetProcAddress(hLib_chess,"PrintBoardG");

	if (pPrintBoardG==NULL || pDllMakeAMove==NULL || pDllGenerateMoves==NULL || pDllStartNewGame==NULL || pDllIsGameOver==NULL || pDllSearch==NULL)
	{
		printf("Can't load procedures needed\n");
		FreeLibrary(hLib_man);
		FreeLibrary(hLib_chess);
		return(1);
	}

   	srand((unsigned) time(&t));

	msecs=10*1000;
	maxdepth=20;

	ngames = 2;

	player[WHITE]=MAN;
	player[BLACK]=AUTO;

	stato=kKEEPSEARCHING;

	for (ng=1;ng<=ngames;ng++)
	{
		printf("\n\nInizio partita nr=%d\n\n\n",ng);
		printf("White plays ");
		if (player[WHITE]==AUTO)
			printf("Auto\n");
		else
			printf("Man\n");
		printf("Black plays ");
		if (player[BLACK]==AUTO)
			printf("Auto\n");
		else
			printf("Man\n");

		(*pDllStartNewGame)("Chess");

		(*pDllMakeAMove)("(White Rook a1");
		(*pDllMakeAMove)("(White Knight b1");
		(*pDllMakeAMove)("(White Bishop c1");
		(*pDllMakeAMove)("(White Queen d1");
		(*pDllMakeAMove)("(White King e1");
		(*pDllMakeAMove)("(White Bishop f1");
		(*pDllMakeAMove)("(White Knight g1");
		(*pDllMakeAMove)("(White Rook h1");

		(*pDllMakeAMove)("(White Pawn a2");
		(*pDllMakeAMove)("(White Pawn b2");
		(*pDllMakeAMove)("(White Pawn c2");
		(*pDllMakeAMove)("(White Pawn d2");
		(*pDllMakeAMove)("(White Pawn e2");
		(*pDllMakeAMove)("(White Pawn f2");
		(*pDllMakeAMove)("(White Pawn g2");
		(*pDllMakeAMove)("(White Pawn h2");

	
		(*pDllMakeAMove)("(Black Rook a8");
		(*pDllMakeAMove)("(Black Knight b8");
		(*pDllMakeAMove)("(Black Bishop c8");
		(*pDllMakeAMove)("(Black Queen d8");
		(*pDllMakeAMove)("(Black King e8");
		(*pDllMakeAMove)("(Black Bishop f8");
		(*pDllMakeAMove)("(Black Knight g8");
		(*pDllMakeAMove)("(Black Rook h8");

		(*pDllMakeAMove)("(Black Pawn a7");
		(*pDllMakeAMove)("(Black Pawn b7");
		(*pDllMakeAMove)("(Black Pawn c7");
		(*pDllMakeAMove)("(Black Pawn d7");
		(*pDllMakeAMove)("(Black Pawn e7");
		(*pDllMakeAMove)("(Black Pawn f7");
		(*pDllMakeAMove)("(Black Pawn g7");
		(*pDllMakeAMove)("(Black Pawn h7");

		side = WHITE;
		nmossa = 1;

		while (1)
		{
			(*pDllIsGameOver)(&lResult,zcomment);
			if (lResult != UNKNOWN_SCORE)
				break;

			if (player[side]==AUTO)
			{
				stop_t=0;
				currentMove[0]='\0';
				iret1 = pthread_create( &thread1, NULL, monitor, NULL);
				(*pDllSearch)(msecs,maxdepth,10,&stato,bestMove,currentMove,&plNodes,&plScore,&plDepth);
				stop_t=1;
				pthread_join( thread1, NULL);
			}
			else
			{
				MoveAllowed();
				strcpy(bestMove,listMoves[rand()%nmoves]);
			}

			(*pDllMakeAMove)(bestMove);

			printf("\n\n");

			(*pPrintBoardG)(1);

			printf("\n");

			if (side==WHITE)
				printf("White: ");
			else
				printf("Black: ");
			printf("Mossa %d ",nmossa);
			printf("- %s\n\n\n",bestMove);

			if (side==WHITE)
				side=BLACK;
			else
			{
				side=WHITE;
				nmossa++;
			}
		}

	printf("Fine partita %d\t\t",ng);
	printf("Numero mosse %d\n",nmossa);

	if (lResult==WIN_SCORE)
		if (side==WHITE)
			printf("White wins!\n");
		else
			printf("Black wins!\n");
	if (lResult==LOSS_SCORE)
		if (side==WHITE)
			printf("White loses!\n");
		else
			printf("Black loses!\n");
	if (lResult==DRAW_SCORE)
		printf("Draw!\n");

	}

	printf("\n\nNumero partite giocate=%d\n\n\n",ngames);

	FreeLibrary(hLib_chess);
	FreeLibrary(hLib_man);

	return(0);
}
