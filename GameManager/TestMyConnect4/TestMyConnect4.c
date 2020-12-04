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

BOOL WINAPI DllMain_GameManager(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain_MyConnect4(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

void SetEngineDefaults(void);

void PrintBoard();

char listMoves[256][32];
int nmoves;

char currentMove[256];

pthread_t thread1;
int  iret1;
int stop_t;

void MoveAllowed(void)
{
	char moveBuffer[32];
	int nm;

        DLL_GenerateMoves(NULL);
        nmoves=0;
        do
                DLL_GenerateMoves(moveBuffer);
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


void main(char argc, char argv[]) {

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

	SetEngineDefaults();

#if defined(_MSC_VER) || defined(__MINGW32__)
        DllMain_GameManager(NULL,DLL_PROCESS_ATTACH,NULL);
        DllMain_MyConnect4(NULL,DLL_PROCESS_ATTACH,NULL);
#else
        DllMain_GameManager(0,DLL_PROCESS_ATTACH,NULL);
        DllMain_MyConnect4(0,DLL_PROCESS_ATTACH,NULL);
#endif

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

		DLL_StartNewGame("Connect4 7x6");

		side = RED;
		nmossa = 1;

		while (1)
		{
			DLL_IsGameOver(&lResult,zcomment);
			if (lResult != UNKNOWN_SCORE)
				break;

			if (player[side]==AUTO)
			{
				stop_t=0;
				currentMove[0]='\0';
				iret1 = pthread_create( &thread1, NULL, monitor, NULL);
				DLL_Search(msecs,maxdepth,10,&stato,bestMove,currentMove,&plNodes,&plScore,&plDepth);
				stop_t=1;
				pthread_join( thread1, NULL);
			}
			else
			{
				MoveAllowed();
				strcpy(bestMove,listMoves[rand()%nmoves]);
			}

			DLL_MakeAMove(bestMove);

			printf("\n\n");

			PrintBoard();

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

	DLL_CleanUp();

}
