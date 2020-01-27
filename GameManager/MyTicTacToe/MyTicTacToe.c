#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "GameManager.h"

#include "MyTicTacToe.h"

#include "Versione.h"


#define STRVER		"\nMyTicTacToe Library --- from Lelli Massimo  --- Versione "VERSIONE"\n\n"


int pos[MAXUDIMS][MAXUDIMS];
U64 PieceKeys[2][MAXUDIMS*MAXUDIMS];
U64 SideKey;
int ply;
int side;
U64 poskey;
int hisPly;

int file_from[MAXDEPTH];
int rank_from[MAXDEPTH];

char pChar[] = { ' ', 'X', 'O' };


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason==DLL_PROCESS_ATTACH)
		GenVers(STRVER);

	return(TRUE); 
}



//
//	Genera la lista delle mosse ammesse per la posizione attuale
//
//	INPUT:		movelist	pointer alla lista delle mosse (out)
//				quiesc		flag quiescenza

int GenMoveAllowed(MList *movelist,int quiesc)
{
	int nranks,nfiles;
	int rank,file;
	int nmoves;

	if (quiesc || CheckFiletto())
	{
		movelist->nmoves=0;
		return 0;
	}

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	nmoves=0;

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++)
			if (pos[rank][file] == EMPTY)
			{
				movelist->m[nmoves].score=0;
				movelist->m[nmoves].bmove[0].from.file=file;
				movelist->m[nmoves].bmove[0].from.rank=rank;
				movelist->m[nmoves].bmove[0].from.piece=MAN;

				movelist->m[nmoves].bmove[0].n_to_moves=0;

				movelist->m[nmoves].flags=PRSPLAY;
				movelist->m[nmoves].nbmoves=1;

				nmoves++;
			}

	movelist->nmoves=nmoves;

	return(0);
}



//
// Log dello stato del board
//

void PrintBoard(void)
{
	int nranks;
	int nfiles;
	int rank;
	int file;
	char str[2];

	str[1] = '\0';

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	DoLog("\n");

	for (file=0;file<nfiles;file++) {
		for (rank=nranks-1;rank>=0;rank--) {

			str[0] = pChar[pos[rank][file]+1];
			DoLog("%s\r",str);
		}

		DoLog("\n");
	}

	DoLog("\n");
}



//
// Torna indietro dall'ultima mossa
//

void TakeBack(void)
{
	int nfiles;
	int file_fr;
	int rank_fr;

	GetDims(FILES,&nfiles);
	
	hisPly--;

	ply--;

	file_fr = file_from[hisPly];
	rank_fr = rank_from[hisPly];

	pos[rank_fr][file_fr] = EMPTY;

	side ^= CIRCLE;
	
	poskey ^= SideKey;

	poskey ^= PieceKeys[side][rank_fr*nfiles+file_fr];

	assert(IsPosOk());
}



//
// Svuota una casella
//
// INPUT:	rank		nr. riga
//			file		nr. colonna

void EmptySquare(int rank,int file)
{
	int nfiles;

	GetDims(FILES,&nfiles);

	if (pos[rank][file] != EMPTY)
		poskey ^= PieceKeys[pos[rank][file]][rank*nfiles+file];

	pos[rank][file] = EMPTY;

	hisPly = 0;

	assert(IsPosOk());
}



//
// Imposta una casella
//
// INPUT:	rank		nr. riga
//			file		nr. colonna
//			player		giocatore
//			p			codice pezzo

void SetSquare(int rank,int file,int player,int p)
{
	int nfiles;

	GetDims(FILES,&nfiles);

	if (pos[rank][file] != EMPTY)
		poskey ^= PieceKeys[pos[rank][file]][rank*nfiles+file];

	pos[rank][file] = player;

	poskey ^= PieceKeys[player][rank*nfiles+file];

	hisPly = 0;

	assert(IsPosOk());
}



//
// Esegue una mossa
//
// INPUT:	move			mossa
//
//	OUTPUT:	0				mossa ammessa
//			<>0				mossa irregolare


int MakeMove(CompMove *move)
{
	int file_fr;
	int rank_fr;
	int nfiles;

	GetDims(FILES,&nfiles);

	file_fr=move->bmove[0].from.file;
	rank_fr=move->bmove[0].from.rank;

	if (pos[rank_fr][file_fr] != EMPTY)
		return(1);

	pos[rank_fr][file_fr] = side;

	file_from[hisPly] = file_fr;
	rank_from[hisPly] = rank_fr;

	hisPly++;

	ply++;

	poskey ^= PieceKeys[side][rank_fr*nfiles+file_fr];

	side ^= CIRCLE;

	poskey ^= SideKey;

	assert(IsPosOk());

	return(0);
}


void CleanUp(void)
{
}

int IsPosOk(void) {
	return ( GeneratePosKey() == poskey );
}

U64 GetHashKey(void) {
	return poskey;
}

void InitSearch(void)
{
	ply = 0;
}

int GetPly(void)
{
	return ply;	
}

int GetSide(void)
{
	return side;
}


//
// Imposta una nuova partita
//

void NewGame(void)
{
	int rank,file;
	int nranks,nfiles;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++)
			pos[rank][file] = EMPTY;

	side = CROSS;

	hisPly = 0;

	poskey = GeneratePosKey();
}


// Check partita finita in parità
//
// OUTPUT:	0		non finita
//			1		finita in parità


int CheckDraw(void)
{
	int rank,file;
	int nranks,nfiles;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++)
			if (pos[rank][file] == EMPTY)
				return(0);

	return(1);
}


int CheckLoss(void) {

	return CheckFiletto();
}

//
// Controlla se il player in input ha posizione vinta
//
//
// OUTPUT:	0		non si sa
//			1		vittoria

int CheckWin(void)
{
	int esito;

	side ^= CIRCLE;
	esito = CheckLoss();
	side ^= CIRCLE;

	return esito;
}

//
// Controlla se il player in input ha posizione persa
//
//
// OUTPUT:	0		non si sa
//			1		sconfitta


int CheckFiletto(void)
{
	int esito;
	int nranks;
	int nfiles;
	int rank;
	int file;
	int opponent;

	opponent = side ^ CIRCLE;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
	{
		esito=1;

		for (file=0;file<nfiles;file++)
			esito = esito && (pos[rank][file] == opponent);

		if (esito)
			break;
	}

	if (esito==0)
		for (file=0;file<nfiles;file++)
		{
			esito=1;

			for (rank=0;rank<nranks;rank++)
				esito = esito && (pos[rank][file] == opponent);

			if (esito)
				break;
		}

	if (esito==0)
	{
		esito=1;

		for (rank=0;rank<nranks;rank++)
			esito = esito && (pos[rank][rank] == opponent);
	}

	if (esito==0)
	{
		esito=1;

		for (rank=0;rank<nranks;rank++)
			esito = esito && (pos[nranks-1-rank][rank] == opponent);
	}

	return (esito);
}



// Funzione di valutazione della posizione
//
// INPUT:	RawEval		flag valutazione grezza
// OUTPUT:	score

int Eval(int RawEval)
{
	int rank,file,count_p,count_o;
	int score;
	int nranks,nfiles;
	int opponent;

	if (CheckFiletto())
		return LOSS_SCORE + ply;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	opponent = side ^ CIRCLE;

	score=0;

	// Controllo pedina su riga vuota

	for (rank=0;rank<nranks;rank++)
	{
		count_p=0;
		count_o=0;

		for (file=0;file<nfiles;file++)
		{
			count_p+=((pos[rank][file] == side) ? 1 : 0);
			count_o+=((pos[rank][file] == opponent) ? 1 : 0);
		}

		if (count_o==0)
			score+=count_p*USCORE;

		if (count_p==0)
			score-=count_o*USCORE;
	}

		// Controllo pedina su riga colonna

	for (file=0;file<nfiles;file++)
	{
		count_p=0;
		count_o=0;

		for (rank=0;rank<nranks;rank++)
		{
			count_p+=((pos[rank][file] == side) ? 1 : 0);
			count_o+=((pos[rank][file] == opponent) ? 1 : 0);
		}

		if (count_o==0)
			score+=count_p*USCORE;

		if (count_p==0)
			score-=count_o*USCORE;
	}

	// Controllo diagonali

	count_p=0;
	count_o=0;

	for (rank=0;rank<nranks;rank++)
	{
		count_p+=((pos[rank][rank] == side) ? 1 : 0);
		count_o+=((pos[rank][rank] == opponent) ? 1 : 0);
	}

	if (count_o==0)
		score+=count_p*USCORE;

	if (count_p==0)
		score-=count_o*USCORE;

	count_p=0;
	count_o=0;

	for (rank=0;rank<nranks;rank++)
	{
		count_p+=((pos[nranks-1-rank][rank] == side) ? 1 : 0);
		count_o+=((pos[nranks-1-rank][rank] == opponent) ? 1 : 0);
	}

	if (count_o==0)
		score+=count_p*USCORE;

	if (count_p==0)
		score-=count_o*USCORE;

	return(score);
}


int InitDll(void) {

	InitHashKeys();

	return TRUE;
}


// Routines interne

//
// Init delle tabelle per il calcolo dell'hash key di una posizione
//

void InitHashKeys(void) {

	int rank,file;
	int nranks,nfiles;
	
	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++) {
			PieceKeys[CROSS][rank*nfiles+file] = RAND_64;
			PieceKeys[CIRCLE][rank*nfiles+file] = RAND_64;
		}

	SideKey = RAND_64;
}


U64 GeneratePosKey(void) {

	int rank,file;
	int nranks,nfiles;
	U64 key;

	key = 0;
	
	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++)
			if (pos[rank][file] != EMPTY)
				key ^= PieceKeys[pos[rank][file]][rank*nfiles+file];

	if (side == CROSS)
		key ^= SideKey;

	return key;
}
