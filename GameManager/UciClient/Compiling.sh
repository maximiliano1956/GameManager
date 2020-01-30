set -x
rm -f UciClient
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyChess/*.c UciClient.c -L. -o UciClient
strip UciClient
