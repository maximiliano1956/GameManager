set -x
cd ../MyChess
rm -f *.o *.a *.dll
x86_64-w64-mingw32-gcc -I../GameManager -D_LIB -c *.c
ar rcs libmychess.a *.o
cd ../GameManager
rm -f *.o *.a *.dll
x86_64-w64-mingw32-gcc -c -D_LIB *.c
ar rcs libgamemanager.a *.o
cd ../UciClient
rm -f *.o uciclient.exe
x86_64-w64-mingw32-gcc -L../GameManager -L../MyChess *.c -lgamemanager -lmychess -lws2_32 -o uciclient.exe
