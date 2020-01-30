set -x
rm -f TestMyTicTacToe
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyTicTacToe/*.c TestMyTicTacToe.c -L. -o TestMyTicTacToe
strip TestMyTicTacToe
