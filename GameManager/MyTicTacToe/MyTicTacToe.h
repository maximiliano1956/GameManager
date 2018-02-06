#define VERSIONE	"1.0"
#define STRVER		"MyTicTacToe Library --- from Lelli Massimo  --- Versione "VERSIONE"\n\n"

#define EMPTY		-1
#define CROSS		0
#define CIRCLE		1

#define MAN			0

#define	USCORE		100

#define RAND_64	(	(U64)rand() | \
					( (U64)rand() << 15 ) | \
					( (U64)rand() << 30 ) | \
					( (U64)rand() << 45 ) | \
					( ( (U64)rand() & 0x0f ) << 60 )	)

void InitHashKeys(void);
U64 GeneratePosKey(void);
int CheckFiletto(void);