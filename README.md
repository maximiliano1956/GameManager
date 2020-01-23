# GameManager

This project has been developed using Visual Studio Community 2015.

Building from sources we obtains:

GameManager.dll

MyChess.dll

MyTicTacToe.dll

UciClient.exe

In the Zrf folder are the two Zillions of games project: MyChess and MyTicTacToe.

You have to copy GameManager.dll and MyChess.dll in MyChess folder

and GameManager.dll and MyTicTacToe.dll in the MyTicTacToe folder.

You can launch MyChess or MyTicTacToe clicking on the .zrf files

or running Zillions of games and loading the .zrf file.

UciClient.exe shares the same engine with MyChess and
is a UCI engine that doesn't need any dll.

Now it's possible cross-compiling having Linux as host (Ubuntu for example)

There are two scripts:

GameManager/CrossCompilingDll.sh create

GameManager/GameManager/GameManager.dll
GameManager/MyChess/MyChess.dll

UciClient/CrossCompiling.sh create UciClient/UciClient.exe



