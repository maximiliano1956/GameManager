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

BOOL WINAPI DllMain_GameManager(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain_MyChess(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

void PrintBoardG(int prf);

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

#if defined(_MSC_VER) || defined(__MINGW32__)
        DllMain_GameManager(NULL,DLL_PROCESS_ATTACH,NULL);
        DllMain_MyChess(NULL,DLL_PROCESS_ATTACH,NULL);
#else
        DllMain_GameManager(0,DLL_PROCESS_ATTACH,NULL);
        DllMain_MyChess(0,DLL_PROCESS_ATTACH,NULL);
#endif

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

		DLL_StartNewGame("Chess");

		DLL_MakeAMove("(White Rook a1");
		DLL_MakeAMove("(White Knight b1");
		DLL_MakeAMove("(White Bishop c1");
		DLL_MakeAMove("(White Queen d1");
		DLL_MakeAMove("(White King e1");
		DLL_MakeAMove("(White Bishop f1");
		DLL_MakeAMove("(White Knight g1");
		DLL_MakeAMove("(White Rook h1");

		DLL_MakeAMove("(White Pawn a2");
		DLL_MakeAMove("(White Pawn b2");
		DLL_MakeAMove("(White Pawn c2");
		DLL_MakeAMove("(White Pawn d2");
		DLL_MakeAMove("(White Pawn e2");
		DLL_MakeAMove("(White Pawn f2");
		DLL_MakeAMove("(White Pawn g2");
		DLL_MakeAMove("(White Pawn h2");

	
		DLL_MakeAMove("(Black Rook a8");
		DLL_MakeAMove("(Black Knight b8");
		DLL_MakeAMove("(Black Bishop c8");
		DLL_MakeAMove("(Black Queen d8");
		DLL_MakeAMove("(Black King e8");
		DLL_MakeAMove("(Black Bishop f8");
		DLL_MakeAMove("(Black Knight g8");
		DLL_MakeAMove("(Black Rook h8");

		DLL_MakeAMove("(Black Pawn a7");
		DLL_MakeAMove("(Black Pawn b7");
		DLL_MakeAMove("(Black Pawn c7");
		DLL_MakeAMove("(Black Pawn d7");
		DLL_MakeAMove("(Black Pawn e7");
		DLL_MakeAMove("(Black Pawn f7");
		DLL_MakeAMove("(Black Pawn g7");
		DLL_MakeAMove("(Black Pawn h7");

		side = WHITE;
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

			PrintBoardG(1);

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

	DLL_CleanUp();

}
