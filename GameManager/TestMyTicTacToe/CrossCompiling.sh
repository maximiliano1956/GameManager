set -x
rm -f TestMyTicTacToe.exe
x86_64-w64-mingw32-gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyTicTacToe/*.c TestMyTicTacToe.c -L. -o TestMyTicTacToe.exe
strip TestMyTicTacToe
