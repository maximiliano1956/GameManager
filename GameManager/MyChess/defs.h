#ifndef DEFS_H								// Per includerlo comunque una sola volta
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#endif

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#ifdef WIN32
#include <winsock2.h>
#endif
#endif


#include <GameManager.h>

#define VERSIONE			"1.0"
#define STRVER				"MyChess Library --- from Lelli Massimo  --- Versione "VERSIONE"\n\n"

#define INPUTBUFFER			( 400 * 6 )		// Lunghezza buffer di input con la GUI (6 e' la lunghezza di una stringa rappresentante una mossa ( 5 char + blank ) )

#define	NAME				"ViceMio"		// Nome engine
#define	BRD_SQ_NUM			120				// Nr. caselle scacchiera "estesa" (2+8+2)*(1+8+1)

#define	MAXGAMEMOVES		2048			// Nr. massimo di mezze mosse in un intero game


// Descrizione notazione fen (Forsyt-Edwards notation)
//
// La posizione viene rappresentata da 6 stringhe separate da uno spazio:
// A B C D E F
//
// A	e' la stringa che descrive la disposizione dei pezzi nella scacchiera.
//		I pezzi bianchi sono rappresentati da lettere maiuscole, i pezzi neri da lettere minuscole.
//		p = Pawn	(pedone)
//		n = kNight	(cavallo)
//		b = Bishop	(alfiere)
//		q = Queen	(regina)
//		k = King	(re)
//
//		La scacchiera e' descritta per righe, dalla ottava riga alla prima riga e dalla colonna A alla colonna H
//		Alla fine di una riga c'e' il carattere "/"; All'inizio dell'ottava riga e alla fine della prima riga non c'e' il carattere "/"
//		n caselle vuote consecutive sono descritte dal numero n (che puo' variare da 1 a 8).
//
//	B	un carattere che descrive chi ha la mossa (w=bianco b=nero)
//
//	C	una stringa che rappresenta i permessi degli arrocchi: se nessun arrocco e' possibile si avra' il carattere "-"
//		altrimenti:
//		K = arrocco corto del bianco possibile
//		Q = arrocco lungo del bianco possibile
//		k = arrocco corto del nero possibile
//		q = arrocco lungo del nero possibile
//
//	D	Descrizione casella en-passant.
//		Se non c'e' nessuna casella en-passnt attiva si ha il carattere "-"
//		Altrimenti si ha una stringa che descrive la casella in notazione algebrica (es: e3)
//
//	E	Contatore per la regola delle 50 mosse
//		Questo contatore si azzera ogni volta che viene mosso un pedone o mangiato un pezzo (da parte del bianco o del nero)
//		In caso contrario viene incrementato. Quando raggiunge il valore 100 (100 semimosse=50 mosse) la partita viene dichiarata patta.
//
//	F	Contatore delle mosse della partita.
//		Si incrementa dopo ogni mossa del nero.
//
//
//
// Posizione iniziale in notazione fen (Forsyt-Edwards notation)
//
#define	START_FEN			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


//
// Definizione codici del tipo di pezzo presente in una casella
//
// EMPTY=0		Nessun pezzo presente
// wP	=1		Pedone bianco
// wN	=2		Cavallo bianco
// wB	=3		Alfiere bianco
// wR	=4		Torre bianco
// wQ	=5		Regina bianco
// wK	=6		Re bianco
// bP	=7		Pedone nero
// bN	=8		Cavallo nero
// bB	=9		Alfiere nero
// bR	=10		Torre nero
// bQ	=11		Regina nero
// bK	=12		Re nero

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };



// Codici delle colonne della scacchiera (notazione algebrica)
//
// Colonna A			codice FILE_A=0
// Colonna B			codice FILE_B=1
// Colonna C			codice FILE_C=2
// Colonna D			codice FILE_D=3
// Colonna E			codice FILE_E=4
// Colonna F			codice FILE_F=5
// Colonna G			codice FILE_G=6
// Colonna H			codice FILE_H=7
// Colonna non valida	codice FILE_NONE=8

enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };

// Codici delle righe della scacchiera (notazione algebrica)
//
// Riga 1			codice RANK_1=0
// Riga 2			codice RANK_2=1
// Riga 3			codice RANK_3=2
// Riga 4			codice RANK_4=3
// Riga 5			codice RANK_5=4
// Riga 6			codice RANK_6=5
// Riga 7			codice RANK_7=6
// Riga 8			codice RANK_8=7
// Riga non valida	codice RANK_NONE=8

enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };


// Codici per il giocatore
//
// Bianco=0
// Nero=1
// Entrambi=2

enum { WHITE, BLACK, BOTH };


// Indici delle casella delle scacchiera reale rispetto quella "estesa"
//
// Indice inteso come indice del vettore rappresentante la scacchiera estesa.
// Il vettore si ottiene accodando le varie righe partendo dalla prima.
//
// Ricordiamo che la scacchiera "estesa" comprende ai bordi delle caselle
// "non valide" che saranno inizializzate una volta per tutte al valore NO_SQ (che vale 99)
// Serve nella generazione delle mosse ammissibili di un pezzo per rendersi conto
// facilmente che siamo andati fuori dalla scacchiera reale.
// Per analizzare una possibile casella target si analizzeranno (in base al tipo di pezzo)
// delle caselle contigue (orizzontalmente o verticalmente o in entrambe le direzioni) a
// quella occupata dal pezzo. Per i pezzi a lungo raggio questa ricerca viene iterata nella
// direzione (cioe' si suppone di avere spostato il pezzo di una casella nella direzione
// di movimento e poi si analizza la casella contigua nella stessa direzione).
// Contornando i bordi della scacchiera reale con una "corona" di caselle non valide,
// saremo sicuri che,indipendentemente dalla casella di partenza,
// non andremo MAI oltre la scacchiera estesa prima di incontrare una casella non valida.
// Potremmo quindi considerare la scacchiera estesa di (1+8+1)*(1+8+1) = 100 caselle.
// Questo sarebbe sufficiente se non ci fosse il Cavallo!
// Il Cavallo non ha traiettoria ma salta. Il salto massimo e' di 2 caselle in ogni direzione
// ortogonale. Se il Cavallo si trova su una fila laterale (A o H) (escludendo le caselle agli angoli)
// qualunque movimento del Cavallo ci fara' rimanere all'interno della scacchiera estesa: muovendoci di 2 caselle
// in verticale (su ' giu') ci porteremmo al piu' sulla prima o ultima riga della scacchiera estesa e il successivo
// movimento sarebbe orizzontale di una sola casella per cui ci manteniamo nei limiti della riga della scacchiera estesa
// Vediamo cosa succede se ci muoviamo di 2 caselle in orizzontale:
// supponiamo ad es. di avere un Cavallo nella colonna H; per andare a destra
// di 2 caselle devo incrementare di 2 l'indice della casella per cui arrivo
// nella scacchiera estesa sulla casella (invalida) della prima colonna della riga successiva.
// Analogamente se il Cavallo nella colonna A e vado a sinistra di 2 caselle devo decrementare di 2
// l'indice della casella per cui arrivo nella scacchiera estesa sull'ultima casella (invalida)
// della riga precedente. Il movimento successivo sara' in verticale di una sola casella per cui ci manteniamo nei limiti
// della colonna della scacchiera estesa.
// Quindi in entrambi i casi mi trovo ancora all'interno della scacchiera estesa.
// Il problema nasce se il Cavallo si trova sulla prima o sull'ultima riga (1 o 8)
// Sulla prima riga se vado in su di 2 caselle devo decrementare l'indice di
// 2*(caselle di una riga) cioe' di 2*10. Ottengo certamente un indice negativo poiche'
// l'indice di un pezzo sulla prima riga della scacchiera reale rispetto quella estesa
// e' compreso tra 11 e 18.
// Sull'ultima riga se vado in giu' di 2 caselle devo invece incrementare l'indice di 2*10.
// Ottengo certamente un indice superiore a 99 perche' l'indice di un pezzo sull'ultima riga
// della scacchiera reale rispetto quella estesa e' compreso tra 81 e 88.
// In entrambi i casi esco comunque dalla scacchiera estesa. (In pratico tento di accedere a 2 righe che non esistono)
// Poiche' nel software la scacchiera estesa viene rappresentata da un vettore in questi
// 2 casi accederei al vettore con un indice scorretto (rispettivamente negativo nel primo
// caso o troppo grande nel secondo caso) e questo farebbe di certo andare in crash il programma.
// Per risolvere il problema e' sufficiente dotare la scacchiera estesa
// di 2 righe aggiuntive (da aggiungere alla prima e accodare all'ultima).
// Ecco spiegato perche' la scacchiera estesa ha 2+8+2 righe e 1+8+1 colonne.
// Quindi la scacchiera estesa sara' un vettore di 120 elementi (120=(2+8+2)*(1+8+1))
//
//     -----------------------------------------
//     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
//     -----------------------------------------
//     |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |20 $21 $22 $23 $24 $25 $26 $27 $28 $29 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |30 $31 $32 $33 $34 $35 $36 $37 $38 $39 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |40 $41 $42 $43 $44 $45 $46 $47 $48 $49 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |50 $51 $52 $53 $54 $55 $56 $57 $58 $59 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |60 $61 $62 $63 $64 $65 $66 $67 $68 $69 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |70 $71 $72 $73 $74 $75 $76 $77 $78 $79 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |80 $81 $82 $83 $84 $85 $86 $87 $88 $89 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |90 $91 $92 $93 $94 $95 $96 $97 $98 $99 |
//     ----$ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $----
//     |100|101|102|103|104|105|106|107|108|109|
//     -----------------------------------------
//     |110|111|112|113|114|115|116|117|118|119|

// NOTA:	NO_SQ verra' utilizzato come identificatore di casella non esistente
//			OFFBOARD verra' utilizzato come identificatore di casella esistente solo nella scacchiera estesa ma non in quella reale

enum {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1 ,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2 ,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3 ,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4 ,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5 ,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6 ,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7 ,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8 , NO_SQ, OFFBOARD
};

#ifndef WIN32 
// Le 2 costanti booleane falso=0 e vero=1

enum { FALSE, TRUE };
#endif

// Maschere per l'accesso ai bit di permesso dell'arrocco
//
// Come stato dei permessi dell'arrocco terremo un intero.
// I primi 2 bit dell'intero si riferiscono al bianco:
// bit 0:	= 1		E' permesso l'arrocco dal lato di Re
// bit 1:	= 1		E' permesso l'arrocco sul lato di Donna
// I successivi 2 bit dell'intero si riferiscono al nero:
// bit 2:	= 1		E' permesso l'arrocco dal lato di Re
// bit 3:	= 1		E' permesso l'arrocco sul lato di Donna
//
// Le costanti che si vanno a definire servono ad isolare uno dei
// primi 4 bit.

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

//
// Struttura per la codifica di una mossa
//
// Lo score servira' per il move ordering in fase di ricerca
//

typedef struct {

	int move;			// Mossa
	int score;			// Punteggio mossa

} S_MOVE;


//
// Struttura che definisce una lista di mosse
// Viene utilizzata per contenere tutte le mosse possibili in una posizione
//

typedef struct {

	S_MOVE	moves[MAXMOVES];					// Array di mosse
	int		count;								// Numero di elementi significativi nell'array

} S_MOVELIST;

//
// Struttura contenente le informazioni necessarie per
// tornare indietro di una mossa nel game
// (per annullare in pratica una mossa)

typedef struct {

	int	move;			// La mossa che vogliamo annullare
	int	castlePerm;		// Stato dei permessi dell'arrocco PRIMA che la mossa da annullare fosse giocata
	int	enPas;			// Casella enpassant PRIMA che la mossa da annullare fosse giocata
	int fiftyMove;		// Contatore mosse per la regola delle 50 mosse PRIMA che la mossa da annullare fosse giocata
	U64 posKey;			// Chiave hash della posizione PRIMA che la mossa da annullare fosse giocata

} S_UNDO;


// Definizione di una struttura contenente le informazioni su uno stato del gioco

// Ricordiamo che per ply si intende una mezza mossa (o del BIANCO o del NERO)
// mentre una mossa e' sempre cosituita da 2 ply (una del BIANCO e una del NERO)

// Concetto di casella enpassant attiva.
//
// Esiste una casella di enpassant attiva solo se la mossa precedente
// e' stata una mossa di pedone di 2 caselle (a partire quindi dalla riga 2 (BIANCO) o 7 (NERO))
// La casella attiva e' quella di passaggio (intermedia tra le caselle di target e di partenza)
// In tutti gli altri casi non ci sara' nessuna casella attiva per l'enpassant.
// Questa informazione serve poiche' se nella posizione attuale (e quindi successiva alla mossa di pedone di 2 caselle
// dell'avversario) c'e' un pedone del giocatore che deve muovere in una delle 2 colonne adiacenti e si trova
// nella riga precedente (mossa del BIANCO) o successiva (mossa del NERO),
// la casella enpassant rappresenta una possibile presa del pedone mosso di 2 caselle da parte del pedone che deve muovere.

// Concetto di bitboard.

// Una bitboard e' un intero a 64 bit dove ogni bit rappresenta un'informazione binaria relativa alla casella corrispondente
// nella scacchiera :	byte 0 --> riga 1
//						byte 1 --> riga 2
//						byte 2 --> riga 3
//						byte 3 --> riga 4
//						byte 4 --> riga 5
//						byte 5 --> riga 6
//						byte 6 --> riga 7
//						byte 7 --> riga 8
//
// Nell'ambito del byte:	bit	0 --> colonna A
//							bit 1 --> colonna B
//							bit 2 --> colonna C
//							bit 3 --> colonna D
//							bit 4 --> colonna E
//							bit 5 --> colonna F
//							bit 6 --> colonna G
//							bit 7 --> colonna H
//
//	In pratica la bitboard e' un vettore di 64 bit dove le singole righe della scacchiera reale
//	vengono accodate (come, ricordiamolo, avviene per il vettore rappresentante la scacchiera estesa)

// Codice hash di una posizione.
//
// E' un indice (peculiare di ogni posizione) che serve come accesso alla sua memorizzazione
// nella tabella delle trasposizioni.


typedef struct {

	int pieces[BRD_SQ_NUM];		// La scacchiera estesa con i vari codici dei pezzi presenti (o assenti)
	U64 pawns[3];				// Vettore di 3 bitboard relative ai pedoni
								// indicizzato con il codice del giocatore (WHITE,BLACK,BOTH)
								// Ogni bit a 1 rappresenta la presenze di un pedone della fazione corrispondente nella
								// casella corrispondente.
								// Questa informazione e' ridondante (la si puo' dedurre dal vettore pieces); pero'
								// queste bitboard facilitano il lavoro nella valutazione della posizione

	int KingSq[2];				// Vettore della posizione nella scacchiera estesa
								// dei Re indicizzato con il codice del giocatore (WHITE,BLACK)
								// Anche questa e' ridondante (la si potrebbe dedurre dal vettore pieces); ma e' comoda
								// per valutare lo scacco al re etc...

	int	side;					// Codice del giocatore che ha la mossa (WHITE o BLACK)
	int enPas;					// Contiene la posizione nella scacchiera estesa relativo alla casella enpassant attiva.
								// Se non c'e' una casella enpassant attiva varra' NO_SQ
	int fiftyMove;				// Contatore mosse per la regola delle 50 mosse

	int ply;					// Il numero di ply della posizione nella ricerca corrente
								// In pratica conta le mezze mosse a partire dalla posizione iniziale della ricerca.

	int hisPly;					// Il numero di ply della posizione nell'ambito dell'intero game
								// In pratica conta le mezze mosse a partire dell'inizio del gioco

	int castlePerm;				// Stato dei permessi per l'arrocco

	U64 posKey;					// Chiave hash relativa alla posizione

	int pceNum[13];				// Vettore indicizzato con il codice di ogni pezzo contenente il numero di pezzi di quel tipo
								// presenti nella posizione
	int bigPce[2];				// Vettore indicizzato con il codice del giocatore contenente il numero di figure
								// del suo schieramento presenti nella posizione
								// (Per figura si intende un pezzo che non sia un pedone)
	int majPce[2];				// Vettore indicizzato con il codice del giocatore contenente il numero di figure maggiori
								// del suo schieramento presenti nella posizione
								// (Per figura maggiore si intende torre o regina)
	int minPce[2];				// Vettore indicizzato con il codice del giocatore contenente il numero di figure minori
								// del suo schieramento presenti nella posizione
								// (Per figura minore si intende cavallo o alfiere)
	int material[2];			// Punteggio relativo al materiale indicizzato con il codice del giocatore

	// Notare che il Re non ha bisogno di essere contato poiche' c'e' sempre in gioco uno e un solo re per schieramento!

	S_UNDO history[MAXGAMEMOVES];	// A questo array di accede con l'indice hisPly e conterra' le informazioni che
									// permetteranno di andare a ritroso nel game. In piu' poiche' contiene i vari codici hash
									// delle varie posizioni, permettera' di individuare agevolmente delle posizioni
									// ripetute (serve nella regola della patta per ripetizione di mosse)

	int pList[13][10];				// Pieces list: contiene informazioni rindondanti sulla posizione, ma che velocizzeranno
									// la generazione di tutte le mosse possibili in una data posizione.
									// Il primo indice e' il codice del pezzo; per ogni codice pezzo si ha una lista
									// di caselle (della scacchiera estesa) occupate da quel tipo di pezzo;
									// la lista e' dimensionata a 10 poiche' nel caso limite si possono
									// avere fino a 10 pezzi dello stesso tipo (es: 2 Torri + 8 pedoni promossi a torre).
									// Il numero di elementi presenti nella lista per il pezzo di codice p sara' pceNum[p]
									// NOTA: Questa matrice non sara' mai utilizzata con il primo indice pari al codice EMPTY (=0)

	int	searchHistory[13][BRD_SQ_NUM];	// Indicizzata per tipo di pezzo e casella: la matrice viene azzerata all'inizio di una ricerca
										// Se lo score della mossa fa' aggiornare lo score della posizione (beats alpha) e la mossa non e' di cattura
										// il valore corrispondente al tipo di pezzo mosso in arrivo nella casella viene incrementato della profondita' corrente nella ricerca

	int searchKillers[2][MAXDEPTH];		// Indicizzato dalla profondita' della ricerca, contiene 2 mosse di non cattura che hanno provocato
										// una "potatura" in beta nella funzione AlphaBeta...
										// In pratica seachKillers mantiene per ogni depth le ultime 2 mosse (in ordine temporale c'e' prima la 1 e poi la 0)
										// che hanno dato beta cutoff

} S_BOARD;


// Tipi di ricerca

#define	STR_ALPHABETA		"AlphaBeta"
#define	STR_MINMAX			"MinMax"
#define STR_NEGAMAX			"NegaMax"


// MACROS

//
// Macro per il calcolo dell'hash della posizione relativamente alle varie informazioni di una posizione
//

#define	HASH_PCE(pce,sq)	( pos->posKey ^= ( PieceKeys[( pce )][( sq )] ) )	// Ex-or della chiave della posizione di cui
																				// pos e' il puntatore con il valore relativo
																				// al pezzo pce nella casella sq

#define	HASH_CA				( pos->posKey ^= ( CastleKeys[( pos->castlePerm )] ) )	// Ex-or della chiave della posizione di cui
																				// pos e' il puntatore con il valore relativo
																				// ai permessi di arrocco

#define	HASH_SIDE			( pos->posKey ^= ( SideKey ) )						// Ex-or della chiave della posizione di cui
																				// pos e' il puntatore con il valore relativo
																				// alla parte interessata contenuta nella
																				// variabile SideKey

#define	HASH_EP				( pos->posKey ^= ( PieceKeys[EMPTY][( pos->enPas )] ) )	// Ex-or della chiave della posizione di cui
																				// pos e' il puntatore con il valore della
																				// casella en-passant dichiarata nella posizione

// Macro per estrarre le varie informazioni dell'intero rappresentante una mossa;
// Ricordiamo che la scacchiera virtuale ha 120 caselle per cui per codificare una
// casella necessiteranno 7 bits (0-127)
// Per codificare il codice pezzo (che va' da 0 a 12) necessiteranno invece 4 bits (0-15)
//
// Vediamo il formato:
//
// Bit 0..6		Casella di partenza
// Bit 7..13	Casella di arrivo
// Bit 14..17	Codice pezzo catturato (0 se non e' una mossa di cattura)
// Bit 18		Settato se e' una mossa di presa en-passant
// Bit 19		Settato se e' la mossa di 2 caselle dalla casella di partenza di un pedone
// Bit 20..23	Codice pezzo promosso (0 se non e' una mossa di promozione)
// Bit 24		Settato se e' una mossa di arrocco
//
// Servono quindi 25 bits: stiamo abbondantemente nel formato di un intero (32 o 64 bits)
//
// Ora vediamo le macro e le maschere:

#define FROMSQ(m)	( (m) & 0x7f )					// Restituisce la casella di partenza della mossa m
#define	TOSQ(m)		( ( (m) >> 7 ) & 0x7f )			// Restituisce la casella di arrivo della mossa m
#define CAPTURED(m)	( ( (m) >> 14 ) & 0x0f )		// Restituisce il codice del pezzo catturato con la mossa m (0 = no cattura)
#define	PROMOTED(m)	( ( (m) >> 20 ) & 0x0f )		// Restituisce il codice del pezzo promosso se c'e' una promozione di pedone
													// con la mossa m (= 0 se no promozione)

#define MFLAGEP		0x040000						// <> 0 la mossa m e' una presa en-passant
#define	MFLAGPS		0x080000						// <> 0 se la mossa m e' una mossa di pedone di 2 dalla casella di partenza
#define	MFLAGCA		0x1000000						// <> 0 se la mossa m e' una mossa di arrocco

#define	MFLAGCAP	0x7C000							// <> 0 se la mossa m e' una mossa di cattura (normale o en-passant)
#define	MFLAGPROM	0xf00000						// <> 0 se la mossa m prevede la promozione di un pedone



// Questa macro calcola l'indice nella scacchiera estesa per una casella di riga r e colonna f nella scacchiera reale.
// Ogni riga in piu' fa' incrementare di 10 l'indice della scacchiera reale, mentre la colonna ha un displacement di 21
// Le parentesi abbondanti servono per evitare i "trappoloni" delle macro. (0<=r<=7) (0<=f<=7)

#define	FR2SQ(f,r)	( (21 + (f) ) + ( (r) * 10 ) )


// Questa macro restituisce l'indice nella bitboard a partire dalla casella della scacchiera estesa

#define SQ64(sq120) (Sq120ToSq64[(sq120)])

// Questa macro restituisce l'indice nella scacchiera estesa a partire dall'indice nella bitboard

#define SQ120(sq64)	(Sq64ToSq120[(sq64)])

// Macro di utility per funzioni di PopBit e CountBits

#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)

// Macro di utility per settare e resettare un bit in una bitboard

#define	CLRBIT(bb,sq) ( (bb) &= ClearMask[(sq)] )
#define	SETBIT(bb,sq) ( (bb) |= SetMask[(sq)] )

// Macro per ricavare il tipo di pezzo

#define	IsBQ(p)		(PieceBishopQueen[(p)])			// TRUE se Alfiere o Regina
#define	IsRQ(p)		(PieceRookQueen[(p)])			// TRUE se Torre o Regina
#define	IsKn(p)		(PieceKnight[(p)])				// TRUE se Cavallo
#define	IsKi(p)		(PieceKing[(p)])				// TRUE se Re

// Questa macro serve per ricavare da una casella nella scacchiera reale,
// quella simmetrica rispetto una linea di simmetria orizzontale che passa
// per il centro scacchera

#define MIRROR64(sq) (Mirror64[ (sq) ])



// GLOBALS

extern	S_BOARD pos[];
extern	int Sq120ToSq64[];
extern	int Sq64ToSq120[];
extern	U64 SetMask[];
extern	U64	ClearMask[];
extern	U64 PieceKeys[][BRD_SQ_NUM];
extern	U64 SideKey;
extern	U64 CastleKeys[];
extern	char *PceChar;
extern	char *SideChar;
extern	char *RankChar;
extern	char *FileChar;
extern int PieceBig[];
extern int PieceMaj[];
extern int PieceMin[];
extern int PieceVal[];
extern int PieceCol[];
extern int FilesBrd[];
extern int RanksBrd[];
extern int PiecePawn[];
extern int PieceKnight[];
extern int PieceKing[];
extern int PieceRookQueen[];
extern int PieceBishopQueen[];
extern int KnDir[];
extern int RkDir[];
extern int BiDir[];
extern int KiDir[];
extern int PieceSlides[];
extern int LoopSlidePce[];
extern int LoopNonSlidePce[];
extern int LoopSlideIndex[];
extern int LoopNonSlideIndex[];
extern int CastlePerm[];
extern int Mirror64[];
extern U64 WhitePassedMask[];
extern U64 BlackPassedMask[];
extern U64 IsolatedMask[];
extern U64 FileBBMask[];
extern U64 RankBBMask[];
extern Search_Status UCI_SearchStatus;
extern char UCI_BestMove[];
extern char UCI_CurrentMove[];
extern long lNodes;
extern long lScore;
extern long lDepth;

// FUNCTIONS

void AllInit(void);
void InitConversionTables(void);
void PrintBitBoard(U64 bb);
int PopBit(U64 *bb);
int CountBits(U64 b);
void InitBitMask(void);
void InitHashKeys(void);
U64 GeneratePosKey(const S_BOARD *pos);
int ParseFen(char *fen);
void ResetBoard(S_BOARD *pos);
void PrintBoard(const S_BOARD *pos);
void PrintBoardUci(void);
void UpdateListsMaterial(S_BOARD *pos);
void InitFilesRanksBrd(void);
int CheckBoard(S_BOARD *pos);
int SqAttacked(int sq,int side);
char *PrSq(int sq);
char *PrMove(int move);
void AddQuietMove(S_BOARD *pos,int move,S_MOVELIST *list);
void AddCaptureMove(S_BOARD *pos,int move,S_MOVELIST *list);
void AddEnPassantMove(S_BOARD *pos,int move,S_MOVELIST *list);
int SqOnBoard(int sq);
int SideValid(int side);
int FileRankValid(int fr);
int PieceValidEmpty(int pce);
int PieceValid(int pce);
void PrintMoveList(S_MOVELIST *list);
void GenerateAllMoves(S_BOARD *pos,S_MOVELIST *list);
void AddWhitePawnCapMove(S_BOARD *pos,int from,int to,int cap,S_MOVELIST *list);
void AddWhitePawnMove(S_BOARD *pos,int from,int to,S_MOVELIST *list);
void AddBlackPawnCapMove(S_BOARD *pos,int from,int to,int cap,S_MOVELIST *list);
void AddBlackPawnMove(S_BOARD *pos,int from,int to,S_MOVELIST *list);
void ClearPiece(int sq, S_BOARD *pos);
void AddPiece(int sq, S_BOARD *pos,int pce);
void MovePiece(int from,int to, S_BOARD *pos);
int MakeMoveV(int move);
void TakeMove(S_BOARD *pos);
void PerftTest(int depth,S_BOARD *pos);
void PerftOne(int depth,char *fenLineWithDepth);
void PerftFile(int depth);
int ParseMove(char *ptrChar);
int IsRepetition(void);
int GetTimeMs();
int GetPvLine(int depth, S_BOARD *pos);
int EvalPosition(int RawEval);
int EvalPositionW(int RawEval);
void InitMvvLva(void);
void PickNextMove(int moveNum,S_MOVELIST *list);
void GenerateAllCaps(S_BOARD *pos,S_MOVELIST *list);
void ParsePosition(char *lineIn);
int ThreeFoldRep(S_BOARD *pos);
int DrawMaterial(S_BOARD *pos);
void InitEvalMasks(void);
int MaterialDraw(S_BOARD *pos);
void MakeNullMove(void);
void TakeNullMove(void);
int MoveListOk(S_MOVELIST *list,S_BOARD *pos);
int SqIs120(int sq);
U64 PolyKeyFromBoard(void);
int HasPawnForCapture(void);
void IniPolyBook(void);
void CleanPolyBook(void);
CompMove GetBookMove(void);
int ConvertPolyToInternalMove(int move);
void ClearForSearch(void);
CompMove ConvMove(int move);
int MoveToInt(CompMove *move);
char *GetMoveStr(CompMove *move);

#endif

