set -x
rm -f UciClient.exe 
x86_64-w64-mingw32-gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyChess/*.c *.c -L. -lws2_32 -o UciClient.exe
strip UciClient.exe
