set -x
cd ../MyChess
rm -f *.o *.a *.dll
gcc -I../GameManager -D_LIB -c *.c
ar rcs libmychess.a *.o
cd ../GameManager
rm -f *.o *.a *.dll
gcc -D_LIB -c *.c
ar rcs libgamemanager.a *.o
cd ../UciClient
rm -f *.o UciClient
gcc -L../GameManager -L../MyChess *.c -lgamemanager -lmychess -o UciClient
