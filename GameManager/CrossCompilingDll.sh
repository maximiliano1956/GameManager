set -x
cd GameManager
rm -f *.o *.a *.dll
i686-w64-mingw32-gcc -shared -municode *.c GameManager_dll.def -o GameManager.dll
cd ../MyChess
rm -f *.o *.a *.dll
i686-w64-mingw32-gcc -I../GameManager -shared -municode *.c ../GameManager/GameManager.dll -o MyChess.dll
