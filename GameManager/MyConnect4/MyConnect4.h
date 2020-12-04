#define EMPTY		-1
#define RED		0
#define BLACK		1

#define MAN		0

#define	USCORE		100

#define RAND_64	(	(U64)rand() | \
					( (U64)rand() << 15 ) | \
					( (U64)rand() << 30 ) | \
					( (U64)rand() << 45 ) | \
					( ( (U64)rand() & 0x0f ) << 60 )	)

void InitHashKeys(void);
U64 GeneratePosKey(void);
int CheckFour(void);
