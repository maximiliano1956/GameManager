#include <windows.h>
#include <stdio.h>

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

	DllMain_GameManager(NULL,DLL_PROCESS_ATTACH,NULL);
	DllMain_MyChess(NULL,DLL_PROCESS_ATTACH,NULL);

	while (TRUE) {
	
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