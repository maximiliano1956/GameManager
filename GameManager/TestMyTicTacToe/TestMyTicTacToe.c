
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#else
#include <string.h>
#endif
#include <stdio.h>

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#undef FAR
#define FAR
#undef PASCAL
#define PASCAL
#undef WINAPI
#define WINAPI
#define LPSTR char *
#define LPVOID void *
#define BOOL unsigned int
#define DWORD unsigned int
#define LPCWSTR const char *
#define LPCSTR const char *
#define HINSTANCE int
#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#define DLL_PROCESS_ATTACH 0
#define INFINITE -1
#endif

typedef enum {
    DLL_OK = 0,
    DLL_OK_DONT_SEND_SETUP = 1, // only supported in 1.0.2 and higher!

    DLL_GENERIC_ERROR = -1,
    DLL_OUT_OF_MEMORY_ERROR = -2,
    DLL_UNKNOWN_VARIANT_ERROR = -3,
    DLL_UNKNOWN_PLAYER_ERROR = -4,
    DLL_UNKNOWN_PIECE_ERROR = -5,
    DLL_WRONG_SIDE_TO_MOVE_ERROR = -6,
    DLL_INVALID_POSITION_ERROR = -7,
    DLL_NO_MOVES = -8
} DLL_Result;


typedef enum {
    kKEEPSEARCHING = 0,
    kSTOPSOON = 1,
    kSTOPNOW = 2
} Search_Status;

BOOL WINAPI DllMain_GameManager(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain_MyTicTacToe(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

DLL_Result FAR PASCAL DLL_Search(long lSearchTime,long lDepthLimit,long lVariety,Search_Status *pSearchStatus,LPSTR bestMove,LPSTR currentMove,long *plNodes,long *plScore,long *plDepth);
DLL_Result FAR PASCAL DLL_MakeAMove(LPCSTR move);
DLL_Result FAR PASCAL DLL_StartNewGame(LPCSTR variant);
DLL_Result FAR PASCAL DLL_CleanUp();
DLL_Result FAR PASCAL DLL_IsGameOver(long *lResult, LPSTR zcomment);
DLL_Result FAR PASCAL DLL_GenerateMoves(LPCSTR moveBuffer);

char listMoves[256][32];
int nmoves;

void MoveAllowed(void)
{
	char moveBuffer[32];
	int nm;

        DLL_GenerateMoves(NULL);
        nmoves=0;
        do
                DLL_GenerateMoves(moveBuffer);
        while (strlen(moveBuffer)!=0 && (strcpy(listMoves[nmoves++],moveBuffer) || 1));

        printf("\n");
        for (nm = 0; nm < nmoves; nm++)
                printf("Mossa %d = %s\n", nm, listMoves[nm]);
        printf("\n");
}


void main(char argc, char argv[]) {

	Search_Status stato;
	char currentMove[256];
	char bestMove[256];
	long plNodes;
	long plScore;
	long plDepth;
	long lResult;
	char zcomment[256];

#if defined(_MSC_VER) || defined(__MINGW32__)
        DllMain_GameManager(NULL,DLL_PROCESS_ATTACH,NULL);
        DllMain_MyTicTacToe(NULL,DLL_PROCESS_ATTACH,NULL);
#else
        DllMain_GameManager(0,DLL_PROCESS_ATTACH,NULL);
        DllMain_MyTicTacToe(0,DLL_PROCESS_ATTACH,NULL);
#endif

	DLL_StartNewGame("MyTicTacToe");

//	DLL_IsGameOver(&lResult,zcomment);

	while (1)
	{
		DLL_Search(2000,10,0,&stato,bestMove,currentMove,&plNodes,&plScore,&plDepth);
		printf("best move =%s\n",bestMove);

		MoveAllowed();

		if (nmoves > 0)
			DLL_MakeAMove(listMoves[0]);
		else
			break;
	}


//	DLL_Search(2000,10,0,&stato,bestMove,currentMove,&plNodes,&plScore,&plDepth);
//	DLL_MakeAMove(bestMove);

//	DLL_IsGameOver(&lResult,zcomment);

	DLL_CleanUp();

}
