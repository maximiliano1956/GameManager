#include "Versione.h"

#include "defs.h"

#define STRVER          "\nMyChess     Library --- from Lelli Massimo  --- Versione "VERSIONE"\n\n"

#define EMPTY_FEN	"8/8/8/8/8/8/8/8 w KQkq - 0 1"

S_BOARD pos[1];								// Posizione

// Variabili per UCI engine 

Search_Status UCI_SearchStatus;
char UCI_BestMove[256];
char UCI_CurrentMove[256];
long lNodes;
long lScore;
long lDepth;

#ifndef _LIB
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#else
BOOL WINAPI DllMain_MyChess(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
#endif
{
	if (fdwReason==DLL_PROCESS_ATTACH)
	{
#ifndef _LIB
		GenVers(STRVER);
#endif
		AllInit();									// Inizializzazioni varie
	}

	return(TRUE); 
}


//
// Routines exportate
//


void CleanUp(void)
{	
	CleanPolyBook();									// Eventuale de-allocazione della ram per la libreria delle aperture
}

void NewGame(void)
{
	ParseFen(EMPTY_FEN);								// Imposta scacchiera vuota

	PrintBoard();
}


//
// Questa funzione inizializza una ricerca e va' chiamata
// ad ogni nuova ricerca
//

void ClearForSearch(void) {

	int index;
	int index2;

	// Azzera le 2 tabelle euristiche searchHistory e searchKillers

	for (index = 0; index < 13; index++)
		for (index2 = 0; index2 < BRD_SQ_NUM; index2++)
			pos->searchHistory[index][index2] = 0;

	for (index = 0; index < 2; index++)
		for (index2 = 0; index2 < MAXDEPTH; index2++)
			pos->searchKillers[index][index2] = 0;

	// Azzera il depth della ricerca

	pos->ply = 0;
}

//
// Questa funzione controlla se la posizione in input e' gia' comparsa
// in precedenza nella history
//
// NOTA: Si suppone che la posizione attuale sia gia' stata memorizzata nella history:
//       sara' percio' l'ultima entry della history.
//		 Ricordando che hisPly e' l'indice alla prima posizione libera nella lista,
//		la ricerca dovra' essere effettuata con un indice che va da 0 a hisPly-1 (poiche'
//		come detto, la posizione attuale e' GIA' stata memorizzata come ultima.
//		In piu' e' possibile fare questo ragionamento. Esistono 2 tipi di mosse
//		che sono irreversibili e sono:
//		a) Quando un pezzo viene mangiato (un pezzo mangiato e' fuori gioco per sempre)
//		b) Quando un pedone viene mosso (poiche' il pedone non puo' tornare indietro)
//		Quindi data la posizione corrente, una posiziona analoga NON puo' essere
//		stata giocata PRIMA di una mossa irreversibile.
//		Quindi la ricerca della posione ripetuta e' sufficiente farla a ritroso
//		dalla coda della history fino alla prima mossa di tipo a) o b).
//		D'altra parte il contatore delle 50 mosse (fiftyMove) conta proprio
//		le mosse a partire dall'ultima spita di pedone o dall'ultima mossa di presa.
//		In conclusione, la ricerca della posizione duale puo' essere fatta per un
//		indice nella history che va' da hisPly-fiftfyMove a hisPly-1
//
// OUTPUT:	TRUE	posizione ripetuta

int IsRepetition(void) {

	int index;		// indice in history

	if (pos->hisPly < 2)
		return FALSE;																	// Almeno 2 mosse eseguite

	for (index = pos->hisPly - pos->fiftyMove; index < pos->hisPly - 1; index++) {		// Loop in history
	
		assert(index >= 0 && index < MAXGAMEMOVES);										// Controllo indice nel vettore history

		if (pos->posKey == pos->history[index].posKey) return TRUE;						// Controllo se la hashkey corrisponde
	}

	return FALSE;
}


int GenMoveAllowed(MList *movelist,int quiesc)
{
	S_MOVELIST list;
	int MoveNum;
	int move;

	if (quiesc)
		GenerateAllCaps(pos,&list);							// Genera la lista delle mosse ammissibili con cattura nella posizione
	else
		GenerateAllMoves(pos,&list);						// Genera la lista delle mosse ammissibili nella posizione

	MoveNum = 0;

	while (MoveNum < list.count)							// Loop per tutte le mosse ammissibili
		if (MakeMoveV(list.moves[MoveNum].move)) {
			TakeMove(pos);									// Si torna indietro alla posizione in input
			MoveNum++;
		}
		else {

			list.count--;

			if (MoveNum == list.count)
				break;
			else
				list.moves[MoveNum].move = list.moves[list.count].move;
			}

	movelist->nmoves = 0;

	for (MoveNum = 0; MoveNum < list.count; MoveNum++) {	// Loop per tutte le mosse legali

		move = list.moves[MoveNum].move;

		movelist->m[MoveNum] = ConvMove(move);

		movelist->m[MoveNum].score = list.moves[MoveNum].score;

		movelist->nmoves++;
	}

	return 0;
}



//
// Torna indietro dall'ultima mossa
//

void TakeBack(void)
{
	TakeMove(pos);
}



//
// Svuota una casella
//
// INPUT:	rank		nr. riga
//			file		nr. colonna

void EmptySquare(int rank,int file)
{
	int sq;

	sq = FR2SQ(file,rank);

	ClearPiece(sq,pos);

	pos->hisPly = 0;

	PrintBoard();
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
	int sq;

	sq = FR2SQ(file,rank);

	if (pos->pieces[sq] != EMPTY)
		ClearPiece(sq,pos);

	AddPiece(sq,pos, player * 6 + p);

	pos->hisPly = 0;

	PrintBoard();
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

	int Move;

	Move = MoveToInt(move);		// Converte la mossa nel formato intero

	if (Move == NOMOVE)
		return 1;

	return (!MakeMoveV(Move));
}


// Check partita finita in parità
//
// OUTPUT:	0		non finita in parita'
//			1		finita in parità


int CheckDraw(void)
{
	int InCheck;
	S_MOVELIST list;
	int MoveNum;

	InCheck = SqAttacked(pos->KingSq[pos->side],pos->side ^ BLACK);		// Valuta se la parte che deve muovere ha il Re in scacco
	
	GenerateAllMoves(pos,&list);							// Genera la lista delle mosse ammissibili nella posizione

	for (MoveNum = 0; MoveNum < list.count; MoveNum++)		// Loop per tutte le mosse ammissibili
		if (MakeMoveV(list.moves[MoveNum].move)) {			// Se la mossa e' effettivamente legale viene eseguita

			TakeMove(pos);									// Si torna indietro alla posizione in input

			return (0);
		}

	return (!InCheck);
}

//
// Controlla se il player in input ha posizione persa
//
//
// OUTPUT:	0		non si sa
//			1		sconfitta


int CheckLoss(void)
{
	int InCheck;
	S_MOVELIST list;
	int MoveNum;

	InCheck = SqAttacked(pos->KingSq[pos->side],pos->side ^ BLACK);		// Valuta se la parte che deve muovere ha il Re in scacco
	
	GenerateAllMoves(pos,&list);							// Genera la lista delle mosse ammissibili nella posizione

	for (MoveNum = 0; MoveNum < list.count; MoveNum++)		// Loop per tutte le mosse ammissibili
		if (MakeMoveV(list.moves[MoveNum].move)) {			// Se la mossa e' effettivamente legale viene eseguita

			TakeMove(pos);									// Si torna indietro alla posizione in input

			return (0);
		}

	return InCheck;
}

//
// Controlla se il player in input ha posizione vinta
//
//
// OUTPUT:	0		non si sa
//			1		vittoria

int CheckWin(void)
{
	return SqAttacked(pos->KingSq[pos->side ^ BLACK],pos->side);
}


int IsPosOk(void) {
	return CheckBoard(pos);
}

int ExtraDepth(void) {
	return SqAttacked(pos->KingSq[pos->side],pos->side ^ BLACK);
};

U64 GetHashKey(void) {
	return pos->posKey;
}

int IsDraw(void) {

return (IsRepetition() || pos->fiftyMove >= 100);			// Test se patta per ripetizione 3 mosse o per regola delle 50 mosse
}

int GetPly(void) {

return pos->ply;
}

int GetSide(void) {

return pos->side;
}

int CanDoNull(void) {

	return ( !SqAttacked(pos->KingSq[pos->side],pos->side ^ BLACK) && (pos->bigPce[pos->side] > 0) && pos->ply );
}

int InitDll(void) {

	return TRUE;
}

int GetPvScore(void) {

	return 2000000;
}

void SetHistory(CompMove move,int depth) {

	int ff;
	int ft;
	int rf;
	int rt;

	ff = move.bmove[0].from.file;
	rf = move.bmove[0].from.rank;
	ft = move.bmove[0].to[0].file;
	rt = move.bmove[0].to[0].rank;

	pos->searchHistory[pos->pieces[FR2SQ(ff,rf)]][FR2SQ(ft,rt)] += depth;	// Incrementa il counter nella tabella,corrispondente al tipo di pezzo mosso e alla casella di arrivo
																			// di un valore pari al depth (in modo da dare punteggi maggiori alle mosse che battono alfa alle maggiori profondita' di ricerca 
}


void SetKillers(CompMove *move) {

	int mossa;

	mossa = MoveToInt(move);

	if (mossa != NOMOVE) {

		pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];		// Shifta la mossa killer di posto 0 (relativa al depth pos-ply) in quella di posto 1 (che viene persa)
		pos->searchKillers[0][pos->ply] =mossa;									// Memorizza la mossa che ha dato beta cutoff nel posto 0
																				// In pratica seachKillers mantiene per ogni depth le ultime 2 mosse (in ordine temporale c'e' prima la 1 e poi la 0)
																				// che hanno dato beta cutoff
	}
}

char *PrintMove(CompMove *move) {

	return GetMoveStr(move);
}




// Funzione di valutazione della posizione
//
// INPUT:	RawEval			Flag per valutazione grezza
// OUTPUT:	score

int Eval(int RawEval)
{
	return ( EvalPosition(RawEval) );
}



//
// Routines interne
//

void InitSearch(void) {

	ClearForSearch();
}


CompMove ConvMove(int move) {

	CompMove mossa;
	int from;
	int to;
	int piece_from;
	int piece_to;
	int piece_prom;
	
	from=FROMSQ(move);
	to=TOSQ(move);

	piece_from = pos->pieces[from];
	piece_to = pos->pieces[to];

	assert(SqOnBoard(from));
	assert(SqOnBoard(to));
	assert(PieceValid(piece_from));
	assert(PieceValidEmpty(piece_to));

	if (pos->side == BLACK) {
		
		piece_from -= 6;
		piece_to -= 6;
	}

	mossa.nbmoves=1;
	mossa.flags=0;

	mossa.bmove[0].n_to_moves=1;
	mossa.bmove[0].from.flags=0;
	mossa.bmove[0].to[0].flags=0;
	mossa.bmove[0].to[1].flags=0;

	mossa.bmove[0].from.file=FilesBrd[from];
	mossa.bmove[0].from.rank=RanksBrd[from];
	mossa.bmove[0].from.piece=piece_from;
	mossa.bmove[0].to[0].file=FilesBrd[to];
	mossa.bmove[0].to[0].rank=RanksBrd[to];
	mossa.bmove[0].to[0].piece=piece_from;

	if (CAPTURED(move))
		mossa.bmove[0].to[0].flags |= CAPTURE;

	if (PROMOTED(move)) {

		piece_prom = PROMOTED(move);

		assert(PieceValid(piece_prom));

		if (pos->side == BLACK)
				piece_prom -= 6;

		mossa.bmove[0].to[0].piece = piece_prom;
	}

	if (move & MFLAGEP) {

		mossa.bmove[0].n_to_moves=2;
		mossa.bmove[0].to[1].file=mossa.bmove[0].to[0].file;
		mossa.bmove[0].to[1].rank = (pos->side == WHITE) ? 4 : 3;
		mossa.bmove[0].to[1].piece = wP;
		mossa.bmove[0].to[1].flags |= CAPTURE;
		mossa.bmove[0].to[0].flags &= ~CAPTURE;
		mossa.bmove[0].n_to_moves=2;
	}

	if (move & MFLAGCA) {

		mossa.bmove[0].from.file=FilesBrd[from];
		mossa.bmove[0].from.rank=RanksBrd[from];
		mossa.bmove[0].from.piece=piece_from;
		mossa.bmove[0].to[0].file=FilesBrd[to];
		mossa.bmove[0].to[0].rank=RanksBrd[to];
		mossa.bmove[0].to[0].piece=piece_from;

		if (to == G1) {
	
			mossa.bmove[1].from.file=7;
			mossa.bmove[1].from.rank=0;
			mossa.bmove[1].from.piece= wR;
			mossa.bmove[1].to[0].file=5;
			mossa.bmove[1].to[0].rank=0;
			mossa.bmove[1].to[0].piece=wR;
		}

		if (to == C1) {
	
			mossa.bmove[1].from.file=0;
			mossa.bmove[1].from.rank=0;
			mossa.bmove[1].from.piece= wR;
			mossa.bmove[1].to[0].file=3;
			mossa.bmove[1].to[0].rank=0;
			mossa.bmove[1].to[0].piece=wR;
		}

		if (to == G8) {
	
			mossa.bmove[1].from.file=7;
			mossa.bmove[1].from.rank=7;
			mossa.bmove[1].from.piece= wR;
			mossa.bmove[1].to[0].file=5;
			mossa.bmove[1].to[0].rank=7;
			mossa.bmove[1].to[0].piece=wR;
		}

		if (to == C8) {
	
			mossa.bmove[1].from.file=0;
			mossa.bmove[1].from.rank=7;
			mossa.bmove[1].from.piece= wR;
			mossa.bmove[1].to[0].file=3;
			mossa.bmove[1].to[0].rank=7;
			mossa.bmove[1].to[0].piece=wR;
		}

		mossa.bmove[1].n_to_moves=1;
		mossa.bmove[1].from.flags=0;
		mossa.bmove[1].to[0].flags=0;
		mossa.nbmoves=2;
	}

	return mossa;
}


//
// Conversione mossa in formato intero
//
// INPUT:	move			mossa
//
//	OUTPUT:					mossa in formato intero (NOMOVE se invalida)


int MoveToInt(CompMove *move)
{

	return ParseMove(GetMoveStr(move));
}



//
// Converte mossa a stringa nel formato interno
//
// INPUT:	move			mossa
//
//	OUTPUT:					stringa mossa nel formato interno


char *GetMoveStr(CompMove *move)
{
	int	ff;			// Colonna casella di partenza
	int fr;			// Traversa casella di partenza
	int tf;			// Colonna casella di arrivo
	int tr;			// Traversa casella di arrivo
	int piece_from;
	int piece_to;
	static char MoveString[6];		// Notazione algebrica della mossa da convertire (es: e7e8q quindi 5 char + fine stringa)

	// Ricava caselle di partenza e arrivo

	ff=move->bmove[0].from.file;
	fr=move->bmove[0].from.rank;
	piece_from=move->bmove[0].from.piece;
	tf=move->bmove[0].to[0].file;
	tr=move->bmove[0].to[0].rank;
	piece_to=move->bmove[0].to[0].piece;

	assert(ff >= 0 && ff <= 7);
	assert(fr >= 0 && fr <= 7);
	assert(tf >= 0 && tf <= 7);
	assert(tr >= 0 && tr <= 7);
	assert(PieceValid(piece_from));
	assert(PieceValid(piece_to));

#if defined(_MSC_VER) || defined(__MINGW32__)
	sprintf_s(MoveString,6,"%c%c%c%c",FileChar[ff],RankChar[fr],FileChar[tf],RankChar[tr]);		// Crea la stringa algebrica senza amcora l'eventuale carattere del pezzo promosso
#else
	sprintf(MoveString,"%c%c%c%c",FileChar[ff],RankChar[fr],FileChar[tf],RankChar[tr]);		// Crea la stringa algebrica senza amcora l'eventuale carattere del pezzo promosso
#endif

	if (move->nbmoves==1 && piece_from!=piece_to)
	{
		MoveString[4] = PceChar[piece_to + 6];
		MoveString[5] = '\0';			// aggiunge alla fine della stringa il carettere di fine stringa
	}

	return MoveString;
}



