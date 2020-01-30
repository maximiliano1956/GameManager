set -x
rm -f TestChess.exe
x86_64-w64-mingw32-gcc -I../../GameManager -D_LIB ../../GameManager/*.c ../../MyChess/*.c TestChess.c -static -g -L. -lpthread -o TestChess.exe
