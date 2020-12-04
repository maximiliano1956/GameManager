set -x
rm -f TestMyConnect4
gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyConnect4/*.c TestMyConnect4.c -L. -lpthread -o TestMyConnect4
strip TestMyConnect4
