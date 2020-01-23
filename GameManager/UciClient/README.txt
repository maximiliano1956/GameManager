UciClient.exe compilato con mingw32 malfunziona se lanciato su un normale xterm con wine o se lanciato con una GUI Linux tramite uno script che usa wine:
il problema e' che in fase di ricerca di una mossa non viene riconosciuta la tastiera, per cui non funziona l'abort della ricerca: bisogna aspettare che finisca
(e questo e' un problema se si e' lanciata un'analisi infinita).
Funziona se utilizzato con wineconsole (vedi script) dando i comandi da terminale; non funziona comunque con una GUI che lo lanci con wineconsole.

Se utilizzato con una GUI lanciata con wine invece funzione perfettamente.
