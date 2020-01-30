#if !defined(_MSC_VER) && !defined(__MINGW32__)
#define FAR
#define PASCAL
#define WINAPI
#define LPSTR char *
#define LPVOID void *
#define BOOL unsigned int
#define DWORD unsigned int
#define LPCSTR const char *
#define HINSTANCE int
#define DLL_PROCESS_ATTACH 0
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

enum {
    UNKNOWN_SCORE = -2140000000L,
    LOSS_SCORE = -2130000000L,
    DRAW_SCORE = 0,
    WIN_SCORE = 2130000000L
};

typedef enum {
    kKEEPSEARCHING = 0,
    kSTOPSOON = 1,
    kSTOPNOW = 2
} Search_Status;

DLL_Result FAR PASCAL DLL_Search(long lSearchTime,long lDepthLimit,long lVariety,Search_Status *pSearchStatus,LPSTR bestMove,LPSTR currentMove,long *plNodes,long *plScore,long *plDepth);
DLL_Result FAR PASCAL DLL_MakeAMove(LPCSTR move);
DLL_Result FAR PASCAL DLL_StartNewGame(LPCSTR variant);
DLL_Result FAR PASCAL DLL_CleanUp();
DLL_Result FAR PASCAL DLL_IsGameOver(long *lResult, LPSTR zcomment);
DLL_Result FAR PASCAL DLL_GenerateMoves(LPCSTR moveBuffer);
