#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <GameManager.h>

#include <extern.h>

#include "MyConnect4.h"

#include "Versione.h"


#define STRVER		"\nMyConnect4 Library --- by Lelli Massimo  --- Versione "VERSIONE"\n\n"

#define MB	256

int pos[MAXUDIMS][MAXUDIMS];
U64 PieceKeys[2][MAXUDIMS*MAXUDIMS];
U64 SideKey;
int ply;
int side;
U64 poskey;
int hisPly;

int file_from[MAXDEPTH];
int rank_from[MAXDEPTH];

char pChar[] = { '.', 'R', 'B' };


#ifndef _LIB
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#else
BOOL WINAPI DllMain_MyConnect4(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#endif
{
#ifndef _LIB
	if (fdwReason==DLL_PROCESS_ATTACH)
		GenVers(STRVER);
#endif

//	SetHashSize(MB);

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

	if (quiesc || CheckFour())
	{
		movelist->nmoves=0;
		return 0;
	}

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	nmoves=0;

	for (file=0;file<nfiles;file++)
		for (rank=0;rank<nranks;rank++)
			if (pos[rank][file] == EMPTY)
			{
				movelist->m[nmoves].score=0;
				movelist->m[nmoves].bmove[0].from.file=file;
				movelist->m[nmoves].bmove[0].from.rank=rank;
				movelist->m[nmoves].bmove[0].from.piece=MAN;
				movelist->m[nmoves].bmove[0].from.flags=0;

				movelist->m[nmoves].bmove[0].n_to_moves=0;

				movelist->m[nmoves].flags=PRSPLAY;
				movelist->m[nmoves].nbmoves=1;

				nmoves++;
				break;
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

	for (rank=nranks-1;rank>=0;rank--) {
		for (file=0;file<nfiles;file++) {

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

	side ^= BLACK;
	
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

	side ^= BLACK;

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
	
	InitHashKeys();
	
	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++)
			pos[rank][file] = EMPTY;

	side = RED;

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

	return CheckFour();
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

	side ^= BLACK;
	esito = CheckLoss();
	side ^= BLACK;

	return esito;
}

//
// Controlla se il player in input ha posizione persa
//
//
// OUTPUT:	0		non si sa
//			1		sconfitta


int CheckFour(void)
{
	int esito;
	int nranks;
	int nfiles;
	int rank;
	int file;
	int rr;
	int ff;
	int opponent;
	int count;

	opponent = side ^ BLACK;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	for (rank=0;rank<nranks;rank++)
	{
		file=0;
		count=0;

		do {

			if (pos[rank][file] == opponent)
			{
				count++;
				if (count==4)
					return(1);
			}
			else
				count=0;

			file++;

		   } while (file<nfiles);
	}

	for (file=0;file<nfiles;file++)
	{
		rank=0;
		count=0;

		do {

			if (pos[rank][file] == opponent)
			{
				count++;
				if (count==4)
					return(1);
			}
			else
				count=0;

			rank++;

		   } while (rank<nranks);
	}

	for (rank=0;rank<nranks;rank++)
		for (file=0;file<nfiles;file++)
			if (pos[rank][file] == opponent)
			{
				rr=rank;
				ff=file;
				count=1;
				while (++rr<nranks && --ff>=0)
				{
					if (pos[rr][ff] == opponent)
					{	
						count++;
						if (count==4)
							return(1);
					}
					else
						count=0;
				}

				rr=rank;
				ff=file;
				count=1;
				while (++rr<nranks && ++ff<nfiles)
				{
					if (pos[rr][ff] == opponent)
					{	
						count++;
						if (count==4)
							return(1);
					}
					else
						count=0;
				}
			}
	
	return (0);
}


int InitDll(void) {

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
			PieceKeys[RED][rank*nfiles+file] = RAND_64;
			PieceKeys[BLACK][rank*nfiles+file] = RAND_64;
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

	if (side == RED)
		key ^= SideKey;

	return key;
}


// Calcolo configurazioni di n checkers consecutivi orizzontali nella posizione
//
// INPUT:	sd		RED/BLACK
// 		n		nr. checkers 2/3/4
// OUTPUT:	numero di condigurazioni orizzontali

int FindHorizChks(int sd,int n)
{
	int nranks,nfiles;
	int rank,file;
	int outc;
	int count;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	outc=0;

	for (rank=0;rank<nranks;rank++)
	{
		count=0;
		for (file=0;file<nfiles;file++)
			if (pos[rank][file]==sd)
			{
				count++;

				if (count==n)
				{
					if (file==(nfiles-1))
						outc++;
					else
					{
						if (pos[rank][file+1]!=sd)
							outc++;
						else
							while (file<nfiles && pos[rank][file]==sd)
								file++;
					}
					count=0;
				}
			}
			else
				count=0;
	}

	return(outc);
}


// Calcolo configurazioni di n checkers consecutivi verticali nella posizione
//
// INPUT:	sd		RED/BLACK
// 		n		nr. checkers 2/3/4
// OUTPUT:	numero di condigurazioni verticali

int FindVertChks(int sd,int n)
{
	int nranks,nfiles;
	int rank,file;
	int outc;
	int count;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	outc=0;

	for (file=0;file<nfiles;file++)
	{
		count=0;
		for (rank=0;rank<nranks;rank++)
			if (pos[rank][file]==sd)
			{
				count++;

				if (count==n)
				{
					if (rank==(nranks-1))
						outc++;
					else
					{
						if (pos[rank+1][file]!=sd)
							outc++;
						else
							while (rank<nranks && pos[rank][file]==sd)
								rank++;
					}
					count=0;
				}
			}
			else
				count=0;
	}

	return(outc);
}

// Calcolo configurazioni di n checkers consecutivi diagonali a destra nella posizione
//
// INPUT:	sd		RED/BLACK
// 		n		nr. checkers 2/3/4
// OUTPUT:	numero di condigurazioni diagonali a destra

int FindDiagRightChks(int sd,int n)
{
	int nranks,nfiles;
	int rank,file;
	int ff;
	int outc;
	int count;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	outc=0;

	for (file=0;file<nfiles-1;file++)
	{
		rank=0;
		ff=file;
		count=0;

		while (rank<nranks && ff<nfiles)
		{
			if (pos[rank][ff] == sd)
			{
				count++;

				if (count==n)
				{
					if (rank==(nranks-1) || ff==(nfiles-1))
						outc++;
					else
					{
						if (pos[rank+1][ff+1]!=sd)
							outc++;
						else
						{
							while (rank<nranks-1 && ff<nfiles-1 && pos[rank][ff]==sd)
							{
								rank++;
								ff++;
							}
						}
					}
					count=0;
				}
			}
			else
				count=0;

			rank++;
			ff++;
		}
	}

	for (file=1;file<nfiles-2;file++)
	{
		rank=nranks-1;
		ff=file;
		count=0;

		while (rank>=0 && ff>=0)
		{
			if (pos[rank][ff] == sd)
			{
				count++;

				if (count==n)
				{
					if (rank==0 || ff==0)
						outc++;
					else
					{
						if (pos[rank-1][ff-1]!=sd)
							outc++;
						else
						{
							while (rank>=0 && ff>=0 && pos[rank][ff]==sd)
							{
								rank--;
								ff--;
							}
						}
					}
					count=0;
				}
			}
			else
				count=0;

			rank--;
			ff--;
		}
	}

	return(outc);
}

// Calcolo configurazioni di n checkers consecutivi diagonali a sinistra nella posizione
//
// INPUT:	sd		RED/BLACK
// 		n		nr. checkers 2/3/4
// OUTPUT:	numero di condigurazioni diagonali a sinistra

int FindDiagLeftChks(int sd,int n)
{
	int nranks,nfiles;
	int rank,file;
	int ff;
	int outc;
	int count;

	GetDims(RANKS,&nranks);
	GetDims(FILES,&nfiles);

	outc=0;

	for (file=1;file<nfiles;file++)
	{
		rank=0;
		ff=file;
		count=0;

		while (rank<nranks && ff>=0)
		{
			if (pos[rank][ff] == sd)
			{
				count++;

				if (count==n)
				{
					if (rank==(nranks-1) || ff==0)
						outc++;
					else
					{
						if (pos[rank+1][ff-1]!=sd)
							outc++;
						else
						{
							while (rank<nranks && ff>=0 && pos[rank][ff]==sd)
							{
								rank++;
								ff--;
							}
						}
					}
					count=0;
				}
			}
			else
				count=0;

			rank++;
			ff--;
		}
	}

	for (file=2;file<nfiles-1;file++)
	{
		rank=nranks-1;
		ff=file;
		count=0;

		while (rank>=0 && ff<nfiles)
		{
			if (pos[rank][file] == sd)
			{
				count++;

				if (count==n)
				{
					if (rank==0 || ff==(nfiles-1))
						outc++;
					else
					{
						if (pos[rank-1][ff+1]!=sd)
							outc++;
						else
						{
							while (rank>=0 && ff<nfiles && pos[rank][ff]==sd)
							{
								rank--;
								ff++;
							}
						}
					}
					count=0;
				}
			}
			else
				count=0;

			rank--;
			ff++;
		}
	}

	return(outc);
}

// Funzione di valutazione della posizione
//
// INPUT:	RawEval		flag valutazione grezza
// OUTPUT:	score

int Eval(int RawEval)
{
	int scoreh_p;
	int scorev_p;
	int scoreh_o;
	int scorev_o;
	int scoredr_p;
	int scoredl_p;
	int scoredr_o;
	int scoredl_o;
	int score;

	scoreh_p=100*FindHorizChks(side,4)+5*FindHorizChks(side,3)+2*FindHorizChks(side,2);
	scorev_p=100*FindVertChks(side,4)+5*FindVertChks(side,3)+2*FindVertChks(side,2);

	scoreh_o=100*FindHorizChks(side^BLACK,4)+5*FindHorizChks(side^BLACK,3)+2*FindHorizChks(side^BLACK,2);
	scorev_o=100*FindVertChks(side^BLACK,4)+5*FindVertChks(side^BLACK,3)+2*FindVertChks(side^BLACK,2);

	scoredl_p=100*FindDiagLeftChks(side,4)+5*FindDiagLeftChks(side,3)+2*FindDiagLeftChks(side,2);
	scoredr_p=100*FindDiagRightChks(side,4)+5*FindDiagRightChks(side,3)+2*FindDiagRightChks(side,2);

	scoredl_o=100*FindDiagLeftChks(side^BLACK,4)+5*FindDiagLeftChks(side^BLACK,3)+2*FindDiagLeftChks(side^BLACK,2);
	scoredr_o=100*FindDiagRightChks(side^BLACK,4)+5*FindDiagRightChks(side^BLACK,3)+2*FindDiagRightChks(side^BLACK,2);

	score = scoreh_p+scorev_p+scoredl_p+scoredr_p-scoreh_o-scorev_o-scoredl_o-scoredr_o;

	return(score);
}

