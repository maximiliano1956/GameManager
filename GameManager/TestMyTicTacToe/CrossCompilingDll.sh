set -x
cd ../GameManager
rm -f ../TestMyTicTacToe/GameManager.dll
i686-w64-mingw32-gcc -shared -municode -DGAMEMANAGER_EXPORTS -DGAME_IMPORTS *.c GameManager_dll.def -o ../TestMyTicTacToe/GameManager.dll
cd ../MyTicTacToe
rm -f ../TestMyTicTacToe/MyTicTacToe.dll
i686-w64-mingw32-gcc -I../GameManager -shared -municode -DGAMEMANAGER_IMPORTS -DGAME_EXPORTS *.c ../TestMyTicTacToe/GameManager.dll -o ../TestMyTicTacToe/MyTicTacToe.dll
cd ../TestMyTicTacToe
rm -f TestMyTicTacToeDll.exe
i686-w64-mingw32-gcc -I../GameManager -mconsole TestMyTicTacToeDll.c -lpthread -static -o TestMyTicTacToeDll.exe
strip TestMyTicTacToeDll.exe

