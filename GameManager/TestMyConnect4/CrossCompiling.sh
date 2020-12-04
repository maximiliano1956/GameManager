set -x
rm -f TestMyConnect4.exe
x86_64-w64-mingw32-gcc -I../GameManager -D_LIB ../GameManager/*.c ../MyConnect4/*.c TestMyConnect4.c -static -L. -lpthread -o TestMyConnect4.exe
strip TestMyConnect4.exe
