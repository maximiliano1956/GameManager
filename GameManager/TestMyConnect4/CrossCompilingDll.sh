set -x
cd ../GameManager
rm -f ../TestMyConnect4/GameManager.dll
i686-w64-mingw32-gcc -shared -municode -DGAMEMANAGER_EXPORTS -DGAME_IMPORTS *.c GameManager_dll.def -o ../TestMyConnect4/GameManager.dll
cd ../MyConnect4
rm -f ../TestMyConnect4/MyConnect4.dll
i686-w64-mingw32-gcc -I../GameManager -shared -municode -DGAMEMANAGER_IMPORTS -DGAME_EXPORTS *.c ../TestMyConnect4/GameManager.dll -o ../TestMyConnect4/MyConnect4.dll
cd ../TestMyConnect4
rm -f TestMyConnect4Dll.exe
i686-w64-mingw32-gcc -I../GameManager -mconsole TestMyConnect4Dll.c -lpthread -static -o TestMyConnect4Dll.exe
strip TestMyConnect4Dll.exe

