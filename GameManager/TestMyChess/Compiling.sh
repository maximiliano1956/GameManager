set -x
rm -f TestMyChess
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyChess/*.c TestMyChess.c -L. -lpthread -o TestMyChess
strip TestMyChess
