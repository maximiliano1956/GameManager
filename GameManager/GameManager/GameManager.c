#if     !defined(_MSC_VER) && !defined(__MINGW32__)
#include <dirent.h>
#endif

#include "Defines.h"
#include "Versione.h"

#define STR_SILOG		"SiLog"
#define OPZ_SILOG		"-"STR_SILOG"="

#define STR_SEARCHTYPE		"SearchType"
#define	STR_RAWEVAL		"RawEval"
#define	STR_USENNUE		"UseNNUE"
#define	STR_QUIESCENCE		"Quiescence"
#define	STR_HASH		"Hash"
#define	STR_NULLMOVE		"NullMove"
#define	STR_BOOK		"Book"


#define OPZ_VERS		"-v"
#define OPZ_HELP		"-h"

#define OPZ_SEARCHTYPE		"-"STR_SEARCHTYPE"="
#define	OPZ_RAWEVAL		"-"STR_RAWEVAL"="
#define	OPZ_USENNUE		"-"STR_USENNUE"="
#define	OPZ_QUIESCENCE		"-"STR_QUIESCENCE"="
#define	OPZ_HASH		"-"STR_HASH"="
#define	OPZ_NULLMOVE		"-"STR_NULLMOVE"="
#define	OPZ_BOOK		"-"STR_BOOK"="


typedef struct
	{
		char par[32];
		char data[512][256];
		int ndato;
	} Stack;

#define STRVER		"\nGameManager Library --- by Lelli Massimo  --- Versione "VERSIONE"\n"

#define EDIT		'('

#define	FILES		0
#define	RANKS		1

#define	MAXVARS		16
#define	MAXPLAYERS	16
#define	MAXPIECES	32
#define	MAXDIMS		8

void GameTerminated(long *score,char *comm);
void DoSearch(long BeginTime,long lSearchTime, long lDepthLimit, long lVariety,Search_Status *pSearchStatus, LPSTR bestMove, LPSTR currentMove,long *plNodes, long *plScore, long *plDepth);
int GetVariation(char *variation);
int ReadGameConfig(char *filename);
int ticBestMoveAlphaBeta(int to_m,int alpha,int beta);
int LoadLibGame(LPCWSTR LibGameName);
void GestConfig(char *inifilename);
int DecodeSquare(char *square,int *file,int *rank);
int Parse(FILE *fp,char *inilin);
char *strtok_mia(char *stringa,char *delim);
int GetNewLineFiltered(FILE *fp);
int CheckStack(char *par);
void GetSquareName(int rank,int file,char *squarename);
void GetPieceName(int npiece,char *piecename);


// obbligatorie
typedef void  (*PNEWGAME)(void);
typedef void  (*PSETSQUARE)(int rank,int file,int player,int p);
typedef void  (*PEMPTYSQUARE)(int rank,int file);
typedef void  (*PCLEANUP)(void);

// opzionale
typedef void  (*PPRINTBOARD)(void);

S_HASHTABLE HashTable;			// Struttura che permette di accedere alla hashtable
S_SEARCHINFO info[1];			// Database ricerca

S_PVARRAY PvArray;				// Vettore che memorizza le mosse della Principal Variation (Variante Principale)

int UciMode = FALSE;


//
// Definizione delle opzioni del motore
//

S_OPTIONS EngineOptions[1];



HANDLE hConsole=INVALID_HANDLE_VALUE;
FILE *fp=NULL;
int	SiLog;

#ifndef _LIB
HMODULE hLib=NULL;
#endif

FILE *fp_parse=NULL;
int initOk=0;

Stack DatPar[128];
int nstack=-1;

char players[MAXVARS][MAXPLAYERS][MAXNAMES+1];
char order[MAXVARS][MAXORDERS][MAXNAMES+1];
char pieces[MAXVARS][MAXPIECES][MAXNAMES+1];
char namedims[MAXVARS][MAXDIMS][MAXUDIMS][MAXNAMES+1];
char variant[MAXVARS][MAXNAMES];

int	nvar;
int npieces[MAXVARS];
int nplayers[MAXVARS];
int nplayers_to_move[MAXVARS];
int ndims[MAXVARS];
int dims[MAXVARS][MAXDIMS];
char appo[MAXUDIMS][MAXNAMES+1];

int numvar;

int flag_piece[MAXVARS];

jmp_buf			mark;
int				BeginTime;
long			SearchTime;
long			lDepth;
LPSTR			CurrMove;
Search_Status	*Search;
LPSTR			Move;
long			*Nodes;
long			*Score;
long			*Depth;

PNEWGAME pNewGame;
PGENMOVEALLOWED pGenMoveAllowed;
PTAKEBACK pTakeBack;
PPRINTBOARD pPrintBoard;
PEVAL pEval;
PCHECKDRAW pCheckDraw;
PCHECKLOSS pCheckLoss;
PCHECKWIN pCheckWin;
PMAKEMOVE pMakeMove;
PSETSQUARE pSetSquare;
PEMPTYSQUARE pEmptySquare;
PCLEANUP pCleanUp;
PISPOSOK pIsPosOk;
PINITSEARCH pInitSearch;
PGETPLY pGetPly;
PGETSIDE pGetSide;
PGETHASHKEY pGetHashKey;

PEXTRADEPTH pExtraDepth = (PEXTRADEPTH)NULL;
PMAKENULLMOVE pMakeNullMove = (PMAKENULLMOVE)NULL;
PTAKENULLMOVE pTakeNullMove = (PTAKENULLMOVE)NULL;
PISDRAW pIsDraw = (PISDRAW)NULL;
PCANDONULL pCanDoNull = (PCANDONULL)NULL;
PGETBOOKMOVE pGetBookMove = (PGETBOOKMOVE)NULL;
PSETHISTORY pSetHistory = (PSETHISTORY)NULL;
PSETKILLERS pSetKillers = (PSETKILLERS)NULL;
PPRINTMOVE pPrintMove = (PPRINTMOVE)NULL;
PINITDLL pInitDll = (PINITDLL)NULL;
PGETPVSCORE pGetPvScore = (PGETPVSCORE)NULL;




// --------------------------------------------------
// Routine per l'interfaccia con Zog o al mio client
// --------------------------------------------------


///
/// FUNZIONI RICHIESTE
///


// DLL_Search
//
// The DLL should search from the current position. If it returns DLL_OK it should
// also return the best move found in str; however, it should not make the move
// internally. A separate call to MakeAMove() will follow to make the move the
// engine returns.
//
// -> lSearchTime: Target search time in milliseconds (0 means forever)
// -> lDepthLimit: Maximum moves deep the engine should search
// -> lVariety: Variety setting for engine. 0 = no variety, 10 = most variety
// -> pSearchStatus: Pointer to variable where Zillions will report search status
// -> bestMove: Pointer to a string where engine can report the best move found so far
// -> currentMove: Pointer to a string where engine can report the move being searched
// -> plNodes: Pointer to a long where engine can report # of positions searched so far
// -> plScore: Pointer to a long where engine can report current best score in search
// -> plDepth: Pointer to a long where engine can report current search depth
//
// Returns DLL_OK or a negative error code 

DLL_Result FAR PASCAL DLL_Search(long lSearchTime,long lDepthLimit,long lVariety,Search_Status *pSearchStatus,LPSTR bestMove,LPSTR currentMove,long *plNodes,long *plScore,long *plDepth)
{
#ifndef _LIB
	DWORD dwThreadId;
	HANDLE hnd;
	DWORD res;
	MSG msg;
#endif

	DoLog("Init search: SearchTime=%d  lDepthLimit=%d lVariety=%d",lSearchTime,lDepthLimit,lVariety);

	// Carica addresses info per la GUI

	info->status = pSearchStatus;	// Address stato della GUI etc....
	info->bMove = bestMove;
	info->currentMove = currentMove;
	info->plNodes = plNodes;
	info->plScore = plScore;
	info->plDepth = plDepth;

	info->timeset = TRUE;								// Gestione tempo attiva
	info->starttime = GetTimeMs();							// Tempo fine ricerca
	info->stoptime = (lSearchTime) ? info->starttime + lSearchTime : INFINITE;	// Tempo di fine ricerca

	info->depth = (lDepthLimit < MAXDEPTH) ? lDepthLimit : MAXDEPTH - 1;		// Setta la profondita' massima della ricerca

#ifndef _LIB
	hnd = CreateThread(NULL,64000000,(LPTHREAD_START_ROUTINE)&SearchPosition,NULL,0,&dwThreadId);

	if (hnd == NULL) {
	
		DoLog("\nCan't create search thread!\n");
		return DLL_OUT_OF_MEMORY_ERROR;
	}

	while (TRUE) {

		res = WaitForSingleObject(hnd,0);

		if (res == WAIT_OBJECT_0)
			break;

		if (!UciMode)
			while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {							// Smaltisce messaggi della GUI

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		Sleep(100);
	}
#else
	SearchPosition();
#endif

	DoLog("\nNodes=%d Best move=%s",*plNodes,bestMove);
	DoLog("\n\n");

	return(DLL_OK);
}

// DLL_MakeAMove
//
// The DLL should try to make the given move internally.
//
// -> move: notation for the move that the engine should make
//
// Returns DLL_OK or a negative error code 

// Mossa ricevuta dalla GUI.
// Se è una mossa di edit e' tra parentesi (viene ustao anche pre il preset iniziale)

// Se invece che una mossa è una casella è perchè dopo verrà chiamata la
// GenerateMoves() per sapere le mosse possibili da quella casella
// Nel caso si tratti di un gioco di drops per cui una casella è indistinguibile da una mossa
// basterà testare se la casella è occupata; se è occupata non e una mossa ma la comunicazione
// della casella di enquiry per la GenerateMoves; se invece e' libera e' una mossa

// Fprmato della mossa

// Puo' essere una mossa:						K a1 - b1			pezzo con sigla K del giocatore che ha il turno dalla casella a1 alla b2
// Puo' essere una mossa con presa:				K a1 x b1			pezzo con sigla K del giocatore che ha il turno dalla casella a1 prende il pezzo in b2 
//             un drop:							X man a1			pezzo con sigla man del giocatore X drop in a1
//			   edit								(White K a1)		pezzo con sigla K del giocatore White in a1

DLL_Result FAR PASCAL DLL_MakeAMove(LPCSTR move)
{
	DoLog("DLL_MakeAMove(%s)",move);

	return (DoMove((char *)move));
}

// DLL_StartNewGame
//
// The DLL should reset the board for a new game.
//
// -> variant: The variant to be played as it appears in the variant menu
//
// Returns DLL_OK, DLL_OK_DONT_SEND_SETUP, DLL_OUT_OF_MEMORY_ERROR, or
//   DLL_GENERIC_ERROR

// Questa routine viene chiamata quando viene selezionata una nuova partita (Pulsante "New Game")
// Oppure al lancio del file .zrf (dopo la chiamata a DllMain()

DLL_Result FAR PASCAL DLL_StartNewGame(LPCSTR variant)
{
	if (initOk==0)
		return(DLL_GENERIC_ERROR);

	nvar=GetVariation((char *)variant);

	if (nvar==-1)
		return(DLL_UNKNOWN_VARIANT_ERROR);

	(*pNewGame)();

	if (variant[0]=='\0')
		DoLog("DLL_StartNewGame(\"\")");
	else
		DoLog("DLL_StartNewGame(%s)",variant);

	return (DLL_OK);
}

// DLL_CleanUp
//
// The DLL should free memory and prepare to be unloaded.
//
// Returns DLL_OK, DLL_OUT_OF_MEMORY_ERROR, or DLL_GENERIC_ERROR

// Questa routine viene chiamata (prima della successiva DllMain() quando viene selezionato un nuovo tipo di gioco (Premendo "Select Game" per cambiare tipo di gioco)
// o all'uscita del programma avendo selezionato un tipo di gioco.

DLL_Result FAR PASCAL DLL_CleanUp()
{
	(*pCleanUp)();								// Chiusura libreria specializzata
	
#if	defined(_MSC_VER) || defined(__MINGW32__)
	if (hConsole!=INVALID_HANDLE_VALUE)
		{
			FreeConsole();
			hConsole=INVALID_HANDLE_VALUE;
		}
#endif
	if (fp)
	{
		fclose(fp);
		fp=NULL;
	}

#ifndef _LIB
	if (hLib)
		FreeLibrary(hLib);
#endif

	if (HashTable.pTable)
		free(HashTable.pTable);				// Rilascia la ram allocata per la tabella hash

	HashTable.pTable = NULL;

	return (DLL_OK);
}


#if	!defined(_MSC_VER) && !defined(__MINGW32__)
void find_ext(char *extn,char *result)
{
        DIR           *d;

        struct dirent *dir;

        result[0]='\0';

        d = opendir(".");

        if (d)
        {
                while ((dir = readdir(d)) != NULL)
                {
                        char *dot;

                        dot = strrchr(dir->d_name, '.');

                        if (!dot || dot == dir->d_name)
                                continue;

                        if (!strcmp(dot + 1, extn))
                                strcpy(result,dir->d_name);
                }

                closedir(d);
        }
}
#endif

//
// FUNZIONI OPZIONALI
//


// Questa routine viene chiamata una sola volta selezionando "Select game" o lanciando il file .zrf

#ifndef _LIB
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#else
BOOL WINAPI DllMain_GameManager(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#endif
{	
	int esito;
	WIN32_FIND_DATAA file_data;
	wchar_t w_libname[128];
	char stringa[128];
	HANDLE pfile;
	char *punt;

	if (fdwReason==DLL_PROCESS_ATTACH)
	{
		if (UciMode)
			GestConfig("UciClient.ini");
		else
			GestConfig("GameManager.ini");
		
#if	defined(_MSC_VER) || defined(__MINGW32__)
		if (SiLog&0x01)
		{
			if (AllocConsole()==0)
				printf("Non riesco ad aprire la console\n");
			else
			{
				hConsole=GetStdHandle(STD_OUTPUT_HANDLE);

				if (hConsole==INVALID_HANDLE_VALUE)
					printf("Non riesco ad aprire la console\n");
			}
		}
#endif
		
		if (SiLog&0x02)
		{
			if (UciMode)
				fp=fopen("UciClient.log","w");
			else
				fp=fopen("GameManager.log","w");
			if (fp==NULL)
				printf("Non riesco a creare il file di log\n");
		}

#ifndef _LIB
		GenVers(STRVER);
#endif
		
#if	defined(_MSC_VER) || defined(__MINGW32__)
		pfile=FindFirstFileA("*.zrf",&file_data);

		if ((pfile!=INVALID_HANDLE_VALUE))
		{
			strcpy(stringa,file_data.cFileName);

			if ((punt=strchr(stringa,'.'))!=NULL)
			{
				*punt='\0';

				MultiByteToWideChar(CP_ACP,0,stringa,-1,w_libname,sizeof(w_libname));
		
				esito=LoadLibGame(w_libname);
			}
			else
				esito=LoadLibGame((LPCWSTR)"");
		}
		else
		{
			strcpy(file_data.cFileName,"");
			esito=LoadLibGame((LPCWSTR)"");
		}
#else
		find_ext("zrf",file_data.cFileName);
		esito=LoadLibGame((LPCWSTR)"");
#endif

		if (esito)
			return(TRUE);

		info->quit = FALSE;							// Init flag forzatura uscita

		if (ReadGameConfig(file_data.cFileName))
			return(TRUE);

		InitHashTable(&HashTable,64);				// Alloca 64 Mb per la ram della hashtable

		initOk=1;

		(*pInitDll)();

		DoLog("DllMain(DLL_PROCESS_ATTACH)");
	}

	return(TRUE); 
}

// DLL_IsGameOver
//
// This optional function is called by Zillions to see if a game is over.  If 
// not present, Zillions uses the goal in the ZRF to decide the winner.
//
// -> lResult: Pointer to the game result which the DLL should fill in when
//              called.  If the game is over the routine should fill in WIN_SCORE,
//              DRAW_SCORE, or LOSS_SCORE.  Otherwise the routine should fill in
//              UNKNOWN_SCORE.
// -> zcomment: Pointer to a 500-char string in Zillions which the DLL can optionally 
//              fill in, to make an announcement about why the game is over, such
//              as "Draw by third repetition".  The DLL should not modify this 
//              string if there is nothing to report.
//
// Returns DLL_OK or a negative error code 

// Questa routine viene chiamata ad ogni alterazione della posizione per sapere se la partita è finita
// e in tal caso qual'è l'esito (win,loose,draw)

DLL_Result FAR PASCAL DLL_IsGameOver(long *lResult, LPSTR zcomment)
{
	GameTerminated(lResult,zcomment);

	if (*lResult==DLL_INVALID_POSITION_ERROR)
	{
		DoLog("DLL_IsGameOver(DLL_INVALID_POSITION_ERROR,\"\")");
		return(DLL_INVALID_POSITION_ERROR);
	}

	if (*lResult==WIN_SCORE)
		DoLog("DLL_IsGameOver(WIN_SCORE,\"\")");
	if (*lResult==LOSS_SCORE)
		DoLog("DLL_IsGameOver(LOSS_SCORE,\"\")");
	if (*lResult==DRAW_SCORE)
		DoLog("DLL_IsGameOver(DRAW_SCORE,\"\")");
	if (*lResult==UNKNOWN_SCORE)
		DoLog("DLL_IsGameOver(UNKNOWN_SCORE,\"\")");

	return(DLL_OK);
}

// Questa routine viene chiamata per informare Zillion su TUTTE le mosse ammesse nella posizione corrente
// Viene chiamata fino a che non viene restituita una stringa vuota. A ogni chiamata viene restituita una mossa.
// La prima volta della sequenza viene chiamata con moveBuffer pari a NULL ma non ho capito perchè!

DLL_Result FAR PASCAL DLL_GenerateMoves(LPCSTR moveBuffer)
{
	static MList movelist;
	static int nmove;

	if (moveBuffer==NULL)
	{
		(*pGenMoveAllowed)(&movelist,FALSE);

		GenStrMoves(&movelist);

		nmove=0;
	}
	else
		if (nmove==movelist.nmoves)
			strcpy((char *)moveBuffer,"");
		else
			strcpy((char *)moveBuffer,movelist.m[nmove++].str_moves);

	return (DLL_OK);
}




/// ------------------------------------------------------------------
/// Routines esportate alla parte specifica del gioco e al mio client
///-------------------------------------------------------------------



//
// Stampa versione
//
// INPUT:	version		versione
//

void GenVers(char *version)
{
	if ((SiLog&0x01)==0 || hConsole==INVALID_HANDLE_VALUE)
		printf("%s",version);
	
	DoLog(version);
}


//
// Gestione log (su console e file)
//
// INPUT:	stesso formato della printf
//

void DoLog(char *str, ...)
{
	DWORD bWritten;
	char stringa[1024];
	char stringa1[1024+1];
	int len;
	va_list args;

	if (SiLog==0)
		return;

	va_start (args, str);
	vsprintf (stringa,str, args);
	va_end (args);

	strcpy(stringa1,stringa);

	len=(int)strlen(stringa1);

	if (stringa1[len-1]=='\r')
		stringa1[len-1]='\0';
	else
		if (stringa1[len-1]!='\n')
			{
				stringa1[len]='\n';
				stringa1[len+1]='\0';
			}
	
#if	defined(_MSC_VER) || defined(__MINGW32__)
	if (SiLog&0x01)
		if (hConsole!=INVALID_HANDLE_VALUE)
			WriteFile(hConsole,stringa1,(DWORD)strlen(stringa1),&bWritten,NULL);
#endif

	if (SiLog&0x01 && !UciMode)
		printf("%s",stringa1);

	if (SiLog&0x02 && fp!=NULL)
	{
		fwrite(stringa1,strlen(stringa1),1,fp);
		fflush(fp);
	}
}



//
// Ricava una dimensione della tavola da gioco
//
// INPUT:	ndim		numero della dimensione
//			dim			(out)

void GetDims(int ndim,int *dim)
{
	*dim=dims[nvar][ndim];
}


//
// Imposta una opzione
//
// INPUT:	Opz			codice opzione
//			Value		valore opzione

void SetOpt(int Opz,int Value) {

	switch (Opz) {

	case BOOK:		EngineOptions->Book = (Value) ? TRUE : FALSE;
						break;
	case SEARCHTYPE:	if (Value < ALPHABETA)
					Value = ALPHABETA;
				if (Value > NEGAMAX)
					Value = NEGAMAX;
				EngineOptions->SearchType = Value;
						break;
	case RAWEVAL:		EngineOptions->RawEval = (Value) ? TRUE : FALSE;
						break;
	case USENNUE:		EngineOptions->UseNNUE = (Value) ? TRUE : FALSE;
						break;	
	case QUIESCENCE:	EngineOptions->Quiescence = (Value) ? TRUE : FALSE;
						break;
	case HASH:		EngineOptions->Hash = (Value) ? TRUE : FALSE;
						break;
	case NULLMOVE:		EngineOptions->NullMove = (Value) ? TRUE : FALSE;
						break;
	}
}


//
// Legge una opzione
//
// INPUT:	Opz			codice opzione
// OUTPUT:	     			valore opzione

int GetOpt(int Opz) {

int Value=0;

switch (Opz) {

	case BOOK:		Value=EngineOptions->Book;
						break;
	case SEARCHTYPE: 	Value=EngineOptions->SearchType;
						break;
	case RAWEVAL:		Value=EngineOptions->RawEval;
						break;
	case USENNUE:		Value=EngineOptions->UseNNUE;
						break;
	case QUIESCENCE:	Value=EngineOptions->Quiescence;
						break;
	case HASH:		Value=EngineOptions->Hash;
						break;
	case NULLMOVE:		Value=EngineOptions->NullMove;
						break;
	}

return(Value);
}

//
// Imposta il size della HashTable
//

void SetHashSize(int HashSize) {

	if (HashSize < 4)
		HashSize = 4;						// Limite inferiore 4 Megabytes
	if (HashSize > MAX_HASH)
		HashSize = MAX_HASH;				// Limite superiore MAX_HASH = 1024 Mbytes ( = 1Gb)

	InitHashTable(&HashTable,HashSize);		// Alloca HashTable
}


//
// Set modalita' UCI
//

void SetUci(void) {
		UciMode = TRUE;
}

//
// Get modalita' UCI
//

int GetUci(void) {

	return UciMode;
}



//
// Legge il file di configurazione del gioco
//
// INPUT:	filename		nome del file di configurazione del gioco
// OUTPUT:	<>0				errore

int ReadGameConfig(char *filename)
{
	int esito;

	nvar=0;
	npieces[0]=0;
	nplayers[0]=0;
	ndims[0]=0;

	fp_parse=fopen(filename,"r");

	if (fp_parse)
	{
		esito=0;
		while (!esito)
			esito=Parse(fp_parse,NULL);
		fclose(fp_parse);
	}
	else
		if (UciMode)
		{
			ndims[0]=2;
			npieces[0]=7;
			nplayers[0]=2;
			nplayers_to_move[0]=2;

			strcpy(variant[0],"Chess");

			strcpy(players[0][0],"White");
			strcpy(players[0][1],"Black");
	
			strcpy(order[0][0],"White");
			strcpy(order[0][1],"Black");
	
			strcpy(pieces[0][0],"Dummy");
			strcpy(pieces[0][1],"Pawn");
			strcpy(pieces[0][2],"Knight");
			strcpy(pieces[0][3],"Bishop");
			strcpy(pieces[0][4],"Rook");
			strcpy(pieces[0][5],"Queen");
			strcpy(pieces[0][6],"King");

			strcpy(namedims[0][FILES][0],"a");
			strcpy(namedims[0][FILES][1],"b");
			strcpy(namedims[0][FILES][2],"c");
			strcpy(namedims[0][FILES][3],"d");
			strcpy(namedims[0][FILES][4],"e");
			strcpy(namedims[0][FILES][5],"f");
			strcpy(namedims[0][FILES][6],"g");
			strcpy(namedims[0][FILES][7],"h");

			strcpy(namedims[0][RANKS][0],"1");
			strcpy(namedims[0][RANKS][1],"2");
			strcpy(namedims[0][RANKS][2],"3");
			strcpy(namedims[0][RANKS][3],"4");
			strcpy(namedims[0][RANKS][4],"5");
			strcpy(namedims[0][RANKS][5],"6");
			strcpy(namedims[0][RANKS][6],"7");
			strcpy(namedims[0][RANKS][7],"8");

			dims[0][FILES]=8;
			dims[0][RANKS]=8;

			nvar=0;
			numvar=1;
		}
		else
		{
			printf("Non riesco a leggere il file %s\n",filename);
			return(1);
		}

	numvar=nvar+1;

	return(0);
}



// ------------------
// Routines interne
// ------------------



char *punlin;
char bLine[256];
char bLine1[256];
int pare=0;


//
// Ricava il nome del pezzo
//
// INPUT:	npiece		numero del pezzo
//			piecename	(out)

void GetPieceName(int npiece,char *piecename)
{
	strcpy(piecename,pieces[nvar][npiece]);
}



//
// Ricava il nome di una casella
//
// INPUT:	movelist	lista mosse
//			player		giocatore che ha la mossa
//			squarename	(out)

void GetSquareName(int rank,int file,char *squarename)
{
	strcpy(squarename,namedims[nvar][FILES][file]);
	strcat(squarename,namedims[nvar][RANKS][rank]);
}


void GenStrMoves(MList *movelist)
{
	int ind;
	int file;
	int rank;
	char square_name[MAXNAMES];
	char str[(MAXNAMES+1)*2];
	char piece_name[MAXNAMES];
	int bmoves;
	int tomoves;

	for (ind=0;ind<movelist->nmoves;ind++)
	{
		movelist->m[ind].str_moves[0]='\0';

		for (bmoves=0;bmoves<movelist->m[ind].nbmoves;bmoves++)
		{
			GetPieceName(movelist->m[ind].bmove[bmoves].from.piece,piece_name);

			if (movelist->m[ind].flags & PRSPLAY)
				sprintf(str,"%s %s ",order[nvar][(*pGetSide)()],piece_name);
			else
				sprintf(str,"%s ",piece_name);

			strcat(movelist->m[ind].str_moves,str);

			file=movelist->m[ind].bmove[bmoves].from.file;
			rank=movelist->m[ind].bmove[bmoves].from.rank;

			GetSquareName(rank,file,square_name);
			strcat(movelist->m[ind].str_moves,square_name);
			
			for (tomoves=0;tomoves<movelist->m[ind].bmove[0].n_to_moves;tomoves++)
			{
				file=movelist->m[ind].bmove[bmoves].to[tomoves].file;
				rank=movelist->m[ind].bmove[bmoves].to[tomoves].rank;

				GetSquareName(rank,file,square_name);

				if (movelist->m[ind].bmove[bmoves].to[tomoves].flags & CAPTURE)
					sprintf(str," x %s",square_name);
				else
					sprintf(str," - %s",square_name);

				strcat(movelist->m[ind].str_moves,str);

				if (movelist->m[ind].bmove[bmoves].from.piece!=movelist->m[ind].bmove[bmoves].to[tomoves].piece)
				{
					GetPieceName(movelist->m[ind].bmove[bmoves].to[tomoves].piece,piece_name);
					strcat(movelist->m[ind].str_moves," = ");
					strcat(movelist->m[ind].str_moves,piece_name);
				}
			}

			if (bmoves<movelist->m[ind].nbmoves-1)
				strcat(movelist->m[ind].str_moves," ");
		}
	}
}


//
// Filtra nuova linea
//
// INPUT:	fp		handle al file
// OUTPUT:	<>0		fine file
//
int GetNewLineFiltered(FILE *fp)
{
	int ind;
	int ind1;
	char c;

	if (fgets(bLine1,sizeof(bLine1),fp)==NULL)
		return(1);

	ind=0;
	ind1=0;

	while ((c=bLine1[ind1++])!='\0' && c!=';')
	{
		if (c==')' && pare==0)
			bLine[ind++]=' ';

		if (c=='"' && pare==0)
			pare=1;
		else
			if (c=='"' && pare==1)
				pare=0;

		if (c==' ' && pare)
			c='@';

		bLine[ind++]=c;
	}

	bLine[ind]='\0';

	punlin=bLine;
	strcpy(bLine1,bLine);

	return(0);
}


//
// Controllo stack
//
// INPUT:	par			paragrafo da trovare
// OUTPUT:	<>0			errore
//
int CheckStack(char *par)
{
	int ns;

	if (nstack==0)
		return(1);

	for (ns=nstack-1;ns>=0;ns--)
	{
		if (!strcmp(DatPar[ns].par,par))
			return(0);

		if (strcmp(DatPar[ns].par,"("))
				return(1);
	}

	return(1);
}

//
// Parse del file .zrf
//
// INPUT:	fp		handle al file
//			inilin	pointer al prossimo token
// OUTPUT:	<>0 errore

int Parse(FILE *fp,char *inilin)
{
	int npl;
	char strdim[128];
	char *pundim;
	int nv;
	int ind;
	int ind1;
	int nd;
	char c;
	
	if (inilin==NULL)
		if (GetNewLineFiltered(fp))
			return(1);

	while ((punlin=strtok(punlin," \t\n"))==NULL || punlin[0]==';')
		if (GetNewLineFiltered(fp))
			return(1);

	if (punlin[0]=='(')
	{
		nstack++;
		strcpy(DatPar[nstack].par,punlin);
		DatPar[nstack].ndato=0;

		if (!strcmp(DatPar[nstack].par,"(game"))
			flag_piece[0]=1;

		if (!strcmp(DatPar[nstack].par,"(dimensions"))
			{
			ndims[nvar]=0;
			memset(dims[nvar],0,MAXDIMS*sizeof(int));
			}
	}

	while (1)
	{
		punlin=NULL;

		while ((punlin=strtok(punlin," \t\n"))==NULL || punlin[0]==';')
			if (GetNewLineFiltered(fp))
				return(1);

		if (!strcmp(punlin,")"))
		{
			
			if (nstack==-1)
				return(0);

			// interpreta dati

			if (!strcmp(DatPar[nstack].par,"(title") && (CheckStack("(game")==0 || CheckStack("(variant")==0))
			{				
				if (DatPar[nstack].ndato!=1)
					return(3);

				if (CheckStack("(variant")==0)
				{
					if (nvar==MAXVARS)
						return(2);

					strcpy(variant[++nvar],DatPar[nstack].data[0]);

					flag_piece[nvar]=1;
				}

				if (CheckStack("(game")==0)
				{
					strcpy(variant[0],DatPar[nstack].data[0]);
					nvar=0;
				}
			}
			else
			if (!strcmp(DatPar[nstack].par,"(players") && (CheckStack("(game")==0 || CheckStack("(variant")==0))
			{
				if (DatPar[nstack].ndato==0)
					return(4);

				for (nplayers[nvar]=0;nplayers[nvar]<DatPar[nstack].ndato;nplayers[nvar]++)
				{
					if (nplayers[nvar]==MAXPLAYERS)
						return(5);

					strcpy(players[nvar][nplayers[nvar]],DatPar[nstack].data[nplayers[nvar]]);
				}
			}
			else
			if (!strcmp(DatPar[nstack].par,"(turn-order") && (CheckStack("(game")==0 || CheckStack("(variant")==0))
			{
				if (DatPar[nstack].ndato==0)
					return(6);

				for (nplayers_to_move[nvar]=0;nplayers_to_move[nvar]<DatPar[nstack].ndato;nplayers_to_move[nvar]++)
				{
					for (npl=0;npl<nplayers[nvar];npl++)
						if (!strcmp(DatPar[nstack].data[nplayers_to_move[nvar]],players[nvar][npl]))
							break;

					if (npl==nplayers[nvar])
						return(8);

					if (nplayers_to_move[nvar]==MAXORDERS)
						return(7);

					strcpy(order[nvar][nplayers_to_move[nvar]],DatPar[nstack].data[nplayers_to_move[nvar]]);
				}
			}
			else
			if (!strcmp(DatPar[nstack].par,"(name") && CheckStack("(piece")==0)
			{
				if (DatPar[nstack].ndato!=1)
					return(9);

				if (flag_piece[nvar])
				{
					flag_piece[nvar]=0;
					npieces[nvar]=0;
				}

				strcpy(pieces[nvar][npieces[nvar]++],DatPar[nstack].data[0]);
			}
			else
			{
				if (CheckStack("(dimensions")==0)
				{
					pundim=DatPar[nstack].par;

					ind=0;
					ind1=0;

					while ((c=pundim[ind++])!='\0')
					{
						if (c!='"' && c!='(')
							strdim[ind1++]=c;
					}

					strdim[ind1]='\0';

					pundim=strdim;

					while ((pundim=strtok_mia(pundim,"/"))!=NULL)								// NON SI PUO' USARE LA STRTOK POICHE' SI ALTEREREBBE LA SUA VARIABILE STATICA INTERNA!!!!!
					{
						strcpy(namedims[nvar][ndims[nvar]][dims[nvar][ndims[nvar]]++],pundim);
						pundim=NULL;
					}
					
					if (ndims[nvar]==1)
					{
						for (ind=0;ind<dims[nvar][ndims[nvar]];ind++)
							strcpy(appo[ind],namedims[nvar][ndims[nvar]][ind]);
						for (ind=0;ind<dims[nvar][ndims[nvar]];ind++)
							strcpy(namedims[nvar][ndims[nvar]][ind],appo[dims[nvar][ndims[nvar]]-1-ind]);
					}

					ndims[nvar]++;
				}
			}

			DoLog("\n");
			DoLog(DatPar[nstack].par);
			for (nd=0;nd<DatPar[nstack].ndato;nd++)
				DoLog(DatPar[nstack].data[nd]);
			DoLog(")");

			if (!strcmp(DatPar[nstack].par,"(game"))
			{
				for (nv=1;nv<MAXVARS;nv++)
				{
					memcpy(players[nv],players[0],MAXPLAYERS*(MAXNAMES+1));
					memcpy(order[nv],order[0],MAXORDERS*(MAXNAMES+1));
					memcpy(pieces[nv],pieces[0],MAXPIECES*(MAXNAMES+1));
					memcpy(namedims[nv],namedims[0],MAXDIMS*MAXUDIMS*(MAXNAMES+1));
					memcpy(dims[nv],dims[0],MAXDIMS*sizeof(int));
					ndims[nv]=ndims[0];
					npieces[nv]=npieces[0];
					nplayers[nv]=nplayers[0];
					nplayers_to_move[nv]=nplayers_to_move[0];
				}
			}

			nstack--;

			return(0);
		}

		if (punlin[0]=='(')
		{
			strcpy(bLine,bLine1);
			if (Parse(fp,punlin))
				break;
		}
		else
		{
			ind=0;
			ind1=0;

			while ((c=punlin[ind++])!='\0')
			{
				if (c=='@')
					c=' ';

				if (c!='"')
					DatPar[nstack].data[DatPar[nstack].ndato][ind1++]=c;
			}

			DatPar[nstack].data[DatPar[nstack].ndato][ind1]='\0';

			DatPar[nstack].ndato++;
		}
	}

	return(1);
}

//
// Identica alla strtok()
//

char *strtok_mia(char *stringa,char *delim)
{
	static char *punt=0;
	char *point;
	int nch;
	int esci;
	char c;

	if (stringa==NULL)
		point=punt;
	else
		point=stringa;

	while (1)
	{
		c=*point;

		if (c=='\0')
			return(NULL);

		for (nch=0;nch<(int)strlen(delim);nch++)
			if (c==delim[nch])
			{
				*point='\0';

				point++;
				break;
			}

		if (nch==(int)strlen(delim))
			break;
	}

	punt=point;
	esci=0;

	while (1)
	{
		c=*punt;

		if (c=='\0')
			break;

		for (nch=0;nch<(int)strlen(delim);nch++)
			if (c==delim[nch])
			{
				*punt='\0';
				punt++;
				esci=1;
				break;
			}

		if (esci)
			break;

		punt++;
	}

	return(point);
}

//
// Lettura file di configurazione di nome inifilename
//
void GestConfig(char *inifilename)
{
	FILE *fp;
	char bLine[64+1];
	char par[32];
	int value;
#ifndef _LIB
	SiLog=0;

	EngineOptions->SearchType = ALPHABETA;		// Per default ricerca tipo AlphaBeta
	EngineOptions->RawEval = FALSE;			// Per default disattiva valutazione grezza
	EngineOptions->UseNNUE = TRUE;                  // Per default attiva la valutazione nnue
	EngineOptions->Quiescence = TRUE;		// Per default ricerca con quiescenza
	EngineOptions->Hash = TRUE;			// Per default gestione hashtable attiva
	EngineOptions->NullMove = TRUE;			// Per default gestione NullMove attiva
	EngineOptions->Book = TRUE;			// Per default gestione Libreria aperture attiva
#endif
	fp=fopen(inifilename,"r");
	
	if (fp==NULL)
		return;

	while (fgets(bLine,64,fp))
	{
		sscanf(bLine,"%s %d",par,&value);

		if (!strcmp(par,STR_SILOG))
			SiLog=value;

		if (!strcmp(par,STR_SEARCHTYPE))
			if (value >= ALPHABETA && value <= NEGAMAX)
				EngineOptions->SearchType = value;
		if (!strcmp(par,STR_RAWEVAL))
			EngineOptions->RawEval = value ? TRUE:FALSE;
		if (!strcmp(par,STR_USENNUE))
			EngineOptions->UseNNUE = value ? TRUE:FALSE;
		if (!strcmp(par,STR_QUIESCENCE))
			EngineOptions->Quiescence = value ? TRUE:FALSE;
		if (!strcmp(par,STR_HASH))
			EngineOptions->Hash = value ? TRUE:FALSE;
		if (!strcmp(par,STR_NULLMOVE))
			EngineOptions->NullMove = value ? TRUE:FALSE;
		if (!strcmp(par,STR_BOOK))
			EngineOptions->Book = value ? TRUE:FALSE;
	}

	fclose(fp);
}


//
// Carica gli address delle funzioni della libreria del gioco
//
//
// INPUT:	LibGameName			nome della libreria
// OUTPUT:	<>0					errore

int LoadLibGame(LPCWSTR LibGameName)
{
#ifndef _LIB
	if ((hLib=LoadLibrary(LibGameName))==NULL)
		return(1);

	pNewGame=(PNEWGAME)GetProcAddress(hLib,"NewGame");
	pGenMoveAllowed=(PGENMOVEALLOWED)GetProcAddress(hLib,"GenMoveAllowed");
	pPrintBoard=(PPRINTBOARD)GetProcAddress(hLib,"PrintBoard");
	pTakeBack=(PTAKEBACK)GetProcAddress(hLib,"TakeBack");
	pEmptySquare=(PEMPTYSQUARE)GetProcAddress(hLib,"EmptySquare");
	pSetSquare=(PSETSQUARE)GetProcAddress(hLib,"SetSquare");
	pMakeMove=(PMAKEMOVE)GetProcAddress(hLib,"MakeMove");
	pCheckDraw=(PCHECKDRAW)GetProcAddress(hLib,"CheckDraw");
	pCheckLoss=(PCHECKLOSS)GetProcAddress(hLib,"CheckLoss");
	pCheckWin=(PCHECKWIN)GetProcAddress(hLib,"CheckWin");
	pEval=(PEVAL)GetProcAddress(hLib,"Eval");
	pCleanUp=(PCLEANUP)GetProcAddress(hLib,"CleanUp");
	pIsPosOk=(PISPOSOK)GetProcAddress(hLib,"IsPosOk");
	pGetHashKey=(PGETHASHKEY)GetProcAddress(hLib,"GetHashKey");
	pInitSearch=(PINITSEARCH)GetProcAddress(hLib,"InitSearch");
	pGetPly=(PGETPLY)GetProcAddress(hLib,"GetPly");
	pGetSide=(PGETSIDE)GetProcAddress(hLib,"GetSide");
	pInitDll=(PGETSIDE)GetProcAddress(hLib,"InitDll");

	pExtraDepth=(PEXTRADEPTH)GetProcAddress(hLib,"ExtraDepth");
	pMakeNullMove=(PMAKENULLMOVE)GetProcAddress(hLib,"MakeNullMove");
	pTakeNullMove=(PTAKENULLMOVE)GetProcAddress(hLib,"TakeNullMove");
	pIsDraw=(PISDRAW)GetProcAddress(hLib,"IsDraw");
	pCanDoNull=(PCANDONULL)GetProcAddress(hLib,"CanDoNull");
	pGetBookMove=(PGETBOOKMOVE)GetProcAddress(hLib,"GetBookMove");
	pSetHistory=(PSETHISTORY)GetProcAddress(hLib,"SetHistory");
	pSetKillers=(PSETKILLERS)GetProcAddress(hLib,"SetKillers");
	pPrintMove=(PPRINTMOVE)GetProcAddress(hLib,"PrintMove");
	pGetPvScore=(PGETPVSCORE)GetProcAddress(hLib,"GetPvScore");
#else
	pNewGame=(PNEWGAME)NewGame;
	pGenMoveAllowed=(PGENMOVEALLOWED)GenMoveAllowed;
	pPrintBoard=(PPRINTBOARD)PrintBoard;
	pTakeBack=(PTAKEBACK)TakeBack;
	pEmptySquare=(PEMPTYSQUARE)EmptySquare;
	pSetSquare=(PSETSQUARE)SetSquare;
	pMakeMove=(PMAKEMOVE)MakeMove;
	pCheckDraw=(PCHECKDRAW)CheckDraw;
	pCheckLoss=(PCHECKLOSS)CheckLoss;
	pCheckWin=(PCHECKWIN)CheckWin;
	pEval=(PEVAL)Eval;
	pCleanUp=(PCLEANUP)CleanUp;
	pIsPosOk=(PISPOSOK)IsPosOk;
	pGetHashKey=(PGETHASHKEY)GetHashKey;
	pInitSearch=(PINITSEARCH)InitSearch;
	pGetPly=(PGETPLY)GetPly;
	pGetSide=(PGETSIDE)GetSide;
	pInitDll=(PGETSIDE)InitDll;
#endif

	if (pNewGame==NULL || pGenMoveAllowed==NULL || pTakeBack==NULL || pEmptySquare==NULL || pSetSquare==NULL || 
		pMakeMove==NULL || pCheckDraw==NULL || pCheckLoss==NULL || pCheckWin==NULL || pEval==NULL || pCleanUp==NULL ||
		pIsPosOk==NULL || pGetHashKey == NULL || pInitSearch == NULL || pGetPly == NULL ||
		pGetSide == NULL || pInitDll == NULL)
		return(1);

return(0);
}


//
// Esecuzione mossa
//
// INPUT:	move						mossa
// OUTPUT:	DLL_OK						mossa corretta
//			DLL_NO_MOVES				mossa non possibile
//			DLL_INVALID_POSITION_ERROR	sigla casella errata
//			DLL_UNKNOWN_PLAYER_ERROR	sigla giocatore errata
//			DLL_UNKNOWN_PIECE_ERROR		sigla pezzo errata

DLL_Result DoMove(char *move)
{
	int rank,file;
	int esito;
	char player[MAXNAMES+1];
	char piece[MAXNAMES+1];
	int npl;
	int nel;
	int np;
	CompMove Move;
	int nplayer;
	int noplayer;
	int nbmoves;
	int n_to_moves;
	char *punt;
	char square[32];
	char oper;
	char piece_to_prom[MAXNAMES+1];

	if (move[0]==EDIT)
	{
		if (move[1]=='x')						// Svuota casella
		{
			esito=DecodeSquare(&move[3],&file,&rank);

			if (esito)
				return(DLL_INVALID_POSITION_ERROR);

			(*pEmptySquare)(rank,file);
		}
		else
		{										// Imposta pezzo in casella

			sscanf(&move[1],"%s %s %s",player,piece,square);

			for (npl=0;npl<nplayers[nvar];npl++)
				if (!strcmp(player,players[nvar][npl]))
					break;

			if (npl==nplayers[nvar])
				return(DLL_UNKNOWN_PLAYER_ERROR);
			else
				nplayer=npl;

			for (np=0;np<npieces[nvar];np++)
				if (!strcmp(piece,pieces[nvar][np]))
					break;

			if (np==npieces[nvar])
				return(DLL_UNKNOWN_PIECE_ERROR);

			esito=DecodeSquare(square,&file,&rank);

			if (esito)
				return(DLL_INVALID_POSITION_ERROR);

			(*pSetSquare)(rank,file,nplayer,np);
		}

		return(DLL_OK);
	}

	punt=move;

	noplayer=0;

	nel=sscanf(punt,"%s",player);

	if (nel==EOF || nel!=1)
		return(DLL_NO_MOVES);

	for (npl=0;npl<nplayers[nvar];npl++)
		if (!strcmp(player,players[nvar][npl]))
			break;

	if (npl<nplayers[nvar])
		punt+=(strlen(player)+1);
	else
	{
		strcpy(player,order[nvar][(*pGetSide)()]);
		noplayer=1;
	}

	Move.flags = (noplayer) ? 0 : PRSPLAY;
	nbmoves=0;

	while (1)
	{
		nel=sscanf(punt," %s %s",piece,square);
		if (nel==EOF)
			break;
		if (nel!=2)
			return(DLL_NO_MOVES);

		for (np=0;np<npieces[nvar];np++)
			if (!strcmp(piece,pieces[nvar][np]))
				break;

		if (np==npieces[nvar])
			return(DLL_UNKNOWN_PIECE_ERROR);

		punt+=(strlen(piece)+1+strlen(square));

		DecodeSquare(square,&file,&rank);

		Move.bmove[nbmoves].from.file=file;
		Move.bmove[nbmoves].from.rank=rank;			
		Move.bmove[nbmoves].from.piece=np;
		Move.bmove[nbmoves].from.flags=0;
		n_to_moves=0;

		while(1)
		{
			piece_to_prom[0]='\0';

			nel=sscanf(punt," - %s = %s",square,piece_to_prom);
			if (nel==EOF)
				if (n_to_moves==0)
				{
					if (Move.flags & PRSPLAY)
						break;
					else
						return(DLL_NO_MOVES);
				}
				else
					break;

			if (nel!=2)
			{
				nel=sscanf(punt," x %s = %s",square,piece_to_prom);
				if (nel!=2)
				{
					nel=sscanf(punt," - %s",square);
					if (nel!=1)
					{
						nel=sscanf(punt," x %s",square);
						if (nel!=1)
						{
							if (n_to_moves==0 && !(Move.flags & PRSPLAY))
								return(DLL_NO_MOVES);
							else
							{
								punt++;
								break;
							}
						}
					}
				}
			}
			
			oper=*(punt+1);

			punt+=(1+1+1+strlen(square));
			if (nel==2)
				punt+=(1+1+1+strlen(piece_to_prom));

			DecodeSquare(square,&file,&rank);

			Move.bmove[nbmoves].to[n_to_moves].file=file;
			Move.bmove[nbmoves].to[n_to_moves].rank=rank;
			Move.bmove[nbmoves].to[n_to_moves].piece=np;
			Move.bmove[nbmoves].to[n_to_moves].flags = (oper=='x') ? CAPTURE : 0;

			if (piece_to_prom[0]!='\0')
			{
				for (np=0;np<npieces[nvar];np++)
					if (!strcmp(piece_to_prom,pieces[nvar][np]))
						break;	

				if (np==npieces[nvar])
					return(DLL_UNKNOWN_PIECE_ERROR);

				Move.bmove[nbmoves].to[n_to_moves].piece=np;
			}

			n_to_moves++;
		}

	Move.bmove[nbmoves].n_to_moves=n_to_moves;

	nbmoves++;
	}

	Move.nbmoves=nbmoves;

	esito=(*pMakeMove)(&Move);
	
	if (esito == DLL_OK && SiLog && pPrintBoard)	       
		(*pPrintBoard)();

	return(esito ? DLL_NO_MOVES : DLL_OK);
}


//
// Test se il gioco e' terminato
//
// OUTPUT:	lResult			LOSS_SCORE						Sconfitta
//							DRAW_SCORE						Parità
//							UNKNOWN_SCORE					La partita non è finita
//							DLL_INVALID_POSITION_ERROR		Posizione errata


void GameTerminated(long *score,char *comm)
{

	comm[0]='\0';

	if (!(*pIsPosOk)())
	{
		*score=DLL_INVALID_POSITION_ERROR;
		return;
	}

	if ((*pCheckLoss)())
	{
		DoLog("Losing position");
		
		if (pPrintBoard)
			(*pPrintBoard)();

		*score=LOSS_SCORE;
	}
	else
		if ((*pCheckWin)())
		{
			DoLog("Winning position");
		
			if (pPrintBoard)
				(*pPrintBoard)();

			*score=WIN_SCORE;
		}
		else
			if ((*pCheckDraw)())
			{
				DoLog("Drawing position");
		
				if (pPrintBoard)
					(*pPrintBoard)();

				*score=DRAW_SCORE;
			}
			else
				*score=UNKNOWN_SCORE;
}


//
// Ricava rank e file da sigla casella
//
// INPUT:	square		sigla casella
//			file		nr. file (out)
//			rank		nr. rank (out)
// OUTPUT:	<>0			sigla errata

int DecodeSquare(char *square,int *file,int *rank)
{
	char *punt;
	int ndim;

	punt=square;

	for (ndim=0;ndim<dims[nvar][FILES];ndim++)
		if (!strncmp(punt,namedims[nvar][FILES][ndim],strlen(namedims[nvar][FILES][ndim])))
		{
			*file=ndim;
			punt+=strlen(namedims[nvar][FILES][ndim]);
			break;
		}

	if (ndim==dims[nvar][FILES])
		return(1);

	for (ndim=0;ndim<dims[nvar][RANKS];ndim++)
		if (!strncmp(punt,namedims[nvar][RANKS][ndim],strlen(namedims[nvar][RANKS][ndim])))
		{
			*rank=ndim;
			break;
		}

	if (ndim==dims[nvar][RANKS])
		return(1);

	return(0);
}


//
// Ricava il numero di variation dal nome
//
// INPUT:	variation		nome della variation
// OUTPUT:					numero della variation (-1 se non esiste)

int GetVariation(char *variation)
{
	int nv;

	for (nv=0;nv<numvar;nv++)
		if (!strcmp(variation,variant[nv]))
			return(nv);

	return(-1);
}


//
// Funzione che verifica se una mossa e' legale in una posizione
//
// Viene utilizzata quando, data una posizione, si ricava una mossa
// memorizzata in hashtable.
// Questa verifica serve per minimizzare (ma non la elimina comunque)
// la possibilita' di avere delle "collisioni", cioe' avere 2
// posizioni diversi che puntano alla stessa entry nell'hashtable
// In tal caso si potrebbe ricavare per una posizione una mossa
// che si riferisce ad un'altra posizione; controllando la legalita'
// della mossa si fa' una cerca di intercettare l'errore
//
// INPUT:	move	mossa da controllare
// OUTPUT:	TRUE	mossa legale
//			FALSE	mossa illegale

int MoveExists(CompMove move) {

	MList list;					// Lista delle mosse ammesse
	int MoveNum;				// Indice nella suddetta lista

	(*pGenMoveAllowed)(&list,FALSE);							// Genera tutte le mosse legali

	for (MoveNum = 0; MoveNum < list.nmoves; MoveNum++)			// Loop su tutte le mosse
		if (CompareMoves(&list.m[MoveNum],&move))	// Verifica se e' uguale alla mossa in input
				return TRUE;						// Se lo e' esce dichiarando che la mossa in input e' legale nella posizione

	return FALSE;													// La mossa in input non e' presente nella lista delle mosse legali: esce dichiarandola percio' illegale
}


int CompareMoves(CompMove *move1,CompMove *move2) {

	int nb;
	int nt;

	if (move1->flags != move2->flags)
		return FALSE;

	if (move1->nbmoves != move2->nbmoves)
		return FALSE;

	for (nb = 0; nb < move1->nbmoves; nb++) {
	
		if (move1->bmove[nb].from.file != move2->bmove[nb].from.file)
			return FALSE;
		if (move1->bmove[nb].from.flags != move2->bmove[nb].from.flags)
			return FALSE;
		if (move1->bmove[nb].from.piece != move2->bmove[nb].from.piece)
			return FALSE;
		if (move1->bmove[nb].from.rank != move2->bmove[nb].from.rank)
			return FALSE;

		if (move1->bmove[nb].n_to_moves != move2->bmove[nb].n_to_moves)
			return FALSE;

		for (nt = 0; nt < move1->bmove[nb].n_to_moves; nt++) {

			if (move1->bmove[nb].to[nt].file != move2->bmove[nb].to[nt].file)
				return FALSE;
			if (move1->bmove[nb].to[nt].flags != move2->bmove[nb].to[nt].flags)
				return FALSE;
			if (move1->bmove[nb].to[nt].piece != move2->bmove[nb].to[nt].piece)
				return FALSE;
			if (move1->bmove[nb].to[nt].rank != move2->bmove[nb].to[nt].rank)
				return FALSE;
		}
	}

	return TRUE;
}

//
// Funzione che restituisce il tempo attuale in millisecondi
//
// Viene differenziata a seconda del sistema WINDOWS/UNIX
//
// OUTPUT:		millisecondi
//

int GetTimeMs() {

#if	defined(_MSC_VER) || defined(__MINGW32__)
	return GetTickCount();
#else
	struct timeval t;

	gettimeofday(&t,NULL);

	return t.tv_sec*1000 + t.tv_usec / 1000;
#endif

}


//
// Questa funzione (trovata su un forum di WinBoard http://home.arcor.de/dreamlike/chess/)
// fornisce un output diverso da zero
// se ci sono dei dati presenti nello standard input
// La funzione non e' bloccante e non scarica i
// dati eventualmente presenti nello standard input
//
// OUTPUT:	<>0		Dati presenti nello standard input

int InputWaiting()
{
#if !defined(_MSC_VER) && !defined(__MINGW32__)
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
   static int init = 0, pipe;
   static HANDLE inh;
   DWORD dw;

   if (!init) {
     init = 1;
     inh = GetStdHandle(STD_INPUT_HANDLE);
     pipe = !GetConsoleMode(inh, &dw);
     if (!pipe) {
        SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(inh);
      }
    }
    if (pipe) {
      if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
      return dw;
    } else {
      GetNumberOfConsoleInputEvents(inh, &dw);
      return dw <= 1 ? 0 : dw;
	}
#endif
}

//
// Visualizzazione Help
//

void ShowHelp(void)
{
	printf("\nCommand line options:\n\n");

	printf("-v\tprint version and quit\n");
	printf("-h\tprint this help and quit\n\n");

	printf("-SiLog=1\tenable console log (works if a GUI is present)\n");
	printf("-SiLog=2\tenable file log\n");
	printf("-SiLog=3\tenable console  (works if a GUI is present) and file log\n\n");

	printf("-SearchType=AlphaBeta\tset search ALFA-BETA\n");
	printf("-SearchType=MinMax\tset search MIN-MAX\n");
	printf("-SearchType=NegaMax\tset search NEGA-MAX\n\n");

	printf("-Hash\tenable hash-table\n");
	printf("-NullMove\tenable null-move\n");
	printf("-RawEval\tenable raw-eval\n");
	printf("-Book\tenable book library (named performance.bin)\n\n\n");
}


//
// Questa funzione imposta le opzioni di default della macchina
//
void SetEngineDefaults(void)
{
SiLog=0;

EngineOptions->SearchType = ALPHABETA;	// Per default ricerca tipo AlphaBeta
EngineOptions->RawEval = FALSE;		// Per default disattiva valutazione grezza
EngineOptions->UseNNUE = TRUE;		// Per default attiva la valutazione nnue
EngineOptions->Quiescence = TRUE;	// Per default ricerca con quiescenza
EngineOptions->Hash = TRUE;		// Per default gestione hashtable attiva
EngineOptions->NullMove = TRUE;		// Per default gestione NullMove attiva
EngineOptions->Book = TRUE;		// Per default gestione Libreria aperture attiva
}


//
// Questa funzione gestisce le opzioni di lancio di Uciclient per settare le variabili di configurazione
//
// OUTPUT: 1 se opzione non riconosciuta  o richiesta stampa help ==> uscire stampando help
// OUTPUT: 2 ==> richiesta stampa versione ==> uscire

int SetOptions(char argc, char *argv[])
{
int ind;
int ValOpz;
char SearchType[128];		// Tipo ricerca
int found;
	
SetEngineDefaults();

found=0;

for (ind=1;ind<argc;ind++)
        {
        	if (!strcmp(argv[ind],OPZ_VERS))
			return(2);

        	if (!strcmp(argv[ind],OPZ_HELP))
		{
			ShowHelp();
			return(3);
		}

        	if (!strncmp(argv[ind],OPZ_SILOG,strlen(OPZ_SILOG)))
                	{
                		sscanf(argv[ind]+strlen(OPZ_SILOG),"%d",&ValOpz);
				if (ValOpz>=0 && ValOpz<=3)
				{
					SiLog = ValOpz;
					found = 1;
				}
                	}

        	if (!strncmp(argv[ind],OPZ_SEARCHTYPE,strlen(OPZ_SEARCHTYPE)))
        	{
                	sscanf(argv[ind]+strlen(OPZ_SEARCHTYPE),"%s",SearchType);

			ValOpz = -1;

			if (!strcmp(SearchType,STR_ALPHABETA))                                          // Decodifica tipi ricerca
				ValOpz = ALPHABETA;

			if (!strcmp(SearchType,STR_MINMAX))
				ValOpz = MINMAX;

			if (!strcmp(SearchType,STR_NEGAMAX))
				ValOpz = NEGAMAX;

			if (ValOpz!=-1)
			{
				SetOpt(SEARCHTYPE,ValOpz);                                              // Setta in maniera opportuna il tipo ricerca
				found = 1;
			}

        	}
        	if (!strncmp(argv[ind],OPZ_QUIESCENCE,strlen(OPZ_QUIESCENCE)))
                	{
                	sscanf(argv[ind]+strlen(OPZ_QUIESCENCE),"%d",&ValOpz);
			SetOpt(QUIESCENCE,ValOpz);
			found = 1;
                	}
        	if (!strncmp(argv[ind],OPZ_HASH,strlen(OPZ_HASH)))
                	{
                	sscanf(argv[ind]+strlen(OPZ_HASH),"%d",&ValOpz);
			SetOpt(HASH,ValOpz);
			found = 1;
                	}
        	if (!strncmp(argv[ind],OPZ_NULLMOVE,strlen(OPZ_NULLMOVE)))
                	{
                	sscanf(argv[ind]+strlen(OPZ_NULLMOVE),"%d",&ValOpz);
			SetOpt(NULLMOVE,ValOpz);
			found = 1;
                	}
        	if (!strncmp(argv[ind],OPZ_RAWEVAL,strlen(OPZ_RAWEVAL)))
                	{
                	sscanf(argv[ind]+strlen(OPZ_RAWEVAL),"%d",&ValOpz);
			SetOpt(RAWEVAL,ValOpz);
			found = 1;
                	}
        	if (!strncmp(argv[ind],OPZ_USENNUE,strlen(OPZ_USENNUE)))
                	{
                	sscanf(argv[ind]+strlen(OPZ_USENNUE),"%d",&ValOpz);
			SetOpt(USENNUE,ValOpz);
			found = 1;
                	}
        	if (!strncmp(argv[ind],OPZ_BOOK,strlen(OPZ_BOOK)))
                	{
                	sscanf(argv[ind]+strlen(OPZ_BOOK),"%d",&ValOpz);
			SetOpt(BOOK,ValOpz);
			found = 1;
                	}
	}

return (argc > 1 && found == 0) ? 1:0;
}

//
// Questa funzione visualizza le opzioni configurate di Uciclient
//

void ShowOptions(void)
{
	char SearchType[128];

	printf("\n\nOptions:\n\n");

	printf(STR_SILOG"=%d\n",SiLog);

	switch (EngineOptions->SearchType)
	{
		case ALPHABETA:	strcpy(SearchType,STR_ALPHABETA);
				break;
		case MINMAX:	strcpy(SearchType,STR_MINMAX);
				break;
		case NEGAMAX:	strcpy(SearchType,STR_NEGAMAX);
				break;
	}

	printf(STR_SEARCHTYPE"=%s\n",SearchType);

	printf(STR_QUIESCENCE"=%d\n",EngineOptions->Quiescence);
	printf(STR_HASH"=%d\n",EngineOptions->Hash);
	printf(STR_NULLMOVE"=%d\n",EngineOptions->NullMove);
	printf(STR_RAWEVAL"=%d\n",EngineOptions->RawEval);
	printf(STR_USENNUE"=%d\n",EngineOptions->UseNNUE);
	printf(STR_BOOK"=%d\n",EngineOptions->Book);

	printf("\n\n");
}
