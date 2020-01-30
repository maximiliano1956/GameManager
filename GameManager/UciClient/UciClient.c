#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#else
#include <string.h>
#endif
#include <stdio.h>

#include <Includes.h>
#include "Versione.h"

#define STRVER          "\nUciClient     Uci engine --- by Lelli Massimo  --- Versione "VERSIONE"\n\n"

BOOL WINAPI DllMain_GameManager(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain_MyChess(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

void SetUci(void);
void Uci_Loop(void);
void GenVers(char *version);

int SetOptions(char argc,char *argv[]);
void ShowOptions(void);
void ShowHelp(void);

void Help(void) {

	printf("Commands: uci quit vers\n\n");
}


void main(char argc,char *argv[]) {

	char line[256];
	int esito;

#if 0
	int wait=1;
	while (wait)
		Sleep(1000);
#endif
	
	SetUci();
	
	esito = SetOptions(argc,argv);
	if (esito!=0)
	{
		if (esito==2)
			GenVers(STRVER);
		if (esito==1)
			ShowHelp();
		return;
	}

	GenVers(STRVER);

	ShowOptions();

	printf("\n\nType help for hints\n\n");
		
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
				
				else if (!strncmp(line,"quit",4))	{	// Uscita
						break;
					}
				
				else if (!strncmp(line,"vers",4))	{	// Versione
						GenVers(STRVER);
						continue;
					}
				
				else 	{	// Comando non riconosciuto
						Help();
						continue;
					}
			}
	}
}
