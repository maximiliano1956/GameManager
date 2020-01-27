set -x
rm -f UciClient
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyChess/*.c *.c -L. -o UciClient
strip UciClient
