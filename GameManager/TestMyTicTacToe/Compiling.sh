set -x
rm -f prova
gcc -I../../GameManager -D_LIB ../../GameManager/*.c ../../MyTicTacToe/*.c main.c -g -L. -o prova
