set -x
rm -f TestMyChess
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyChess/*.c TestChess.c -g -L. -lpthread -o TestMyChess
