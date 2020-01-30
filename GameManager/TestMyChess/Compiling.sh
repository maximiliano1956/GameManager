set -x
rm -f TestMyChess
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyChess/*.c TestMyChess.c -g -L. -lpthread -o TestMyChess
