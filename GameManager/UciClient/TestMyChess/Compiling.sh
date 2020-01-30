set -x
rm -f TestChess
gcc -I../../GameManager -D_LIB ../../GameManager/*.c ../../MyChess/*.c TestChess.c -g -L. -lpthread -o TestChess
