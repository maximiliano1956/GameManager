#include "Defines.h"


//
// Funzione che restituisce il tempo attuale in millisecondi
//
// Viene differenziata a seconda del sistema WINDOWS/UNIX
//
// OUTPUT:		millisecondi
//

int GetTimeMs() {

#ifdef _MSC_VER
	return GetTickCount();
#else
	struct timeval t;

	gettimeofday(&t,NULL);

	return t.tv_sec*1000 + t.tv_usec / 1000;
#endif

}
