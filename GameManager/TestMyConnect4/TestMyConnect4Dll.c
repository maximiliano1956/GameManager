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

#define BLACK	0
#define RED	1

#define	MAN	0
#define AUTO	1


typedef void (*PPRINTBOARD)();

typedef DLL_Result FAR PASCAL (*PDLLMAKEAMOVE)(LPCSTR move);
typedef DLL_Result FAR PASCAL (*PDLLGENERATEMOVES)(LPCSTR moveBuffer);
typedef DLL_Result FAR PASCAL (*PDLLSTARTNEWGAME)(LPCSTR variant);
typedef DLL_Result FAR PASCAL (*PDLLISGAMEOVER)(long *lResult, LPSTR zcomment);
typedef DLL_Result FAR PASCAL (*PDLLSEARCH)(long lSearchTime,long lDepthLimit,long lVariety,Search_Status *pSearchStatus,LPSTR bestMove,LPSTR currentMove,long *plNodes,long *plScore,long *plDepth);
typedef DLL_Result FAR PASCAL (*PDLLCLEANUP)();

PPRINTBOARD pPrintBoard;
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
HMODULE hLib_myconnect4=NULL;



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

        if ((hLib_myconnect4=LoadLibrary("MyConnect4"))==NULL)
        {
                printf("Can't load MyConnect4.dll\n");
                FreeLibrary(hLib_man);
                return(1);
        }

        pDllMakeAMove=(PDLLMAKEAMOVE)GetProcAddress(hLib_man,"DLL_MakeAMove");
        pDllGenerateMoves=(PDLLGENERATEMOVES)GetProcAddress(hLib_man,"DLL_GenerateMoves");
        pDllStartNewGame=(PDLLSTARTNEWGAME)GetProcAddress(hLib_man,"DLL_StartNewGame");
        pDllIsGameOver=(PDLLISGAMEOVER)GetProcAddress(hLib_man,"DLL_IsGameOver");
        pDllSearch=(PDLLSEARCH)GetProcAddress(hLib_man,"DLL_Search");

        pPrintBoard=(PPRINTBOARD)GetProcAddress(hLib_myconnect4,"PrintBoard");

        if (pPrintBoard==NULL || pDllMakeAMove==NULL || pDllGenerateMoves==NULL || pDllStartNewGame==NULL || pDllIsGameOver==NULL || pDllSearch==NULL)
        {
                printf("Can't load procedures needed\n");
                FreeLibrary(hLib_man);
                FreeLibrary(hLib_myconnect4);
                return(1);
        }


   	srand((unsigned) time(&t));

	msecs=10*1000;
	maxdepth=9;

	ngames = 2;

	player[BLACK]=MAN;
	player[RED]=AUTO;

	stato=kKEEPSEARCHING;

	for (ng=1;ng<=ngames;ng++)
	{
		printf("\n\nInizio partita nr=%d\n\n\n",ng);
		printf("Black plays ");
		if (player[BLACK]==AUTO)
			printf("Auto\n");
		else
			printf("Man\n");
		printf("Red plays ");
		if (player[RED]==AUTO)
			printf("Auto\n");
		else
			printf("Man\n");

		(*pDllStartNewGame)("Connect4 7x6");

		side = RED;
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

			(*pPrintBoard)();

			printf("\n");

			if (side==BLACK)
				printf("Black: ");
			else
				printf("Red: ");

			printf("Mossa %d ",nmossa);
			printf("- %s\n\n\n",bestMove);

			if (side==BLACK)
				side=RED;
			else
			{
				side=BLACK;
				nmossa++;
			}
		}

	printf("Fine partita %d\t\t",ng);
	printf("Numero mosse %d\n",nmossa);

	if (lResult==WIN_SCORE)
		if (side==BLACK)
			printf("Black wins!\n");
		else
			printf("Red wins!\n");
	if (lResult==LOSS_SCORE)
		if (side==BLACK)
			printf("Black loses!\n");
		else
			printf("Red loses!\n");
	if (lResult==DRAW_SCORE)
		printf("Draw!\n");

	}

	printf("\n\nNumero partite giocate=%d\n\n\n",ngames);

        FreeLibrary(hLib_myconnect4);
        FreeLibrary(hLib_man);

	return(0);
}
