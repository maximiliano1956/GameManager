set -x
cd ../GameManager
rm -f GameManager.dll
i686-w64-mingw32-gcc -shared -municode -DGAMEMANAGER_EXPORTS -DGAME_IMPORTS *.c GameManager_dll.def -o GameManager.dll
cd ../MyChess
rm -f MyChess.dll
i686-w64-mingw32-gcc -I../GameManager -shared -municode -DGAMEMANAGER_IMPORTS -DGAME_EXPORTS *.c ../GameManager/GameManager.dll -o MyChess.dll
cd ../TestMyChess
rm -f TestMyChessDll.exe
i686-w64-mingw32-gcc -I../GameManager -mconsole TestMyChessDll.c -lpthread -static -o TestMyChessDll.exe
cp ../GameManager/GameManager.dll .
cp ../MyChess/MyChess.dll .
strip TestMyChessDll.exe