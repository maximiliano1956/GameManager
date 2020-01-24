#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#else
#include <string.h>
#endif
#include <stdio.h>


#if !defined(_MSC_VER) && !defined(__MINGW32__)
#define DLL_PROCESS_ATTACH 0
#define LPVOID void *
#define BOOL unsigned int
#define DWORD unsigned int
#define HINSTANCE int
#undef WINAPI
#define WINAPI
#endif

BOOL WINAPI DllMain_GameManager(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain_MyChess(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

void Uci_Loop(void);


void main(void) {

	char line[256];

#if 0
	int wait=1;
	while (wait)
		Sleep(1000);
#endif
	
#if defined(_MSC_VER) || defined(__MINGW32__)
	DllMain_GameManager(NULL,DLL_PROCESS_ATTACH,NULL);
	DllMain_MyChess(NULL,DLL_PROCESS_ATTACH,NULL);
#else
	DllMain_GameManager(0,DLL_PROCESS_ATTACH,NULL);
	DllMain_MyChess(0,DLL_PROCESS_ATTACH,NULL);
#endif

	while (1) {
	
		memset(line,0,sizeof(line));			// Azzera il buffer in input;

		fflush(stdout);							// Propedeutico

		if (fgets(line,sizeof(line),stdin))		// Se c'e' qualcosa in input
			if (line[0] != '\n') {				// e non e' una linea vuota
			
				if (!strncmp(line,"uci",3))	{	// Set protocollo UCI
					Uci_Loop();
					break;
				}
			}
	}
}
