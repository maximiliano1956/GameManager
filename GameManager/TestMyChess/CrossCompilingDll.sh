set -x
cd ../GameManager
rm -f ../TestMyChess/GameManager.dll
i686-w64-mingw32-gcc -shared -municode -DGAMEMANAGER_EXPORTS -DGAME_IMPORTS *.c GameManager_dll.def -o ../TestMyChess/GameManager.dll
cd ../MyChess
rm -f ../TestMyChess/MyChess.dll
i686-w64-mingw32-gcc -I../GameManager -shared -municode -DGAMEMANAGER_IMPORTS -DGAME_EXPORTS *.c ../TestMyChess/GameManager.dll -o ../TestMyChess/MyChess.dll
cd ../TestMyChess
rm -f TestMyChessDll.exe
i686-w64-mingw32-gcc -I../GameManager -mconsole TestMyChessDll.c -lpthread -static -o TestMyChessDll.exe
strip TestMyChessDll.exe
