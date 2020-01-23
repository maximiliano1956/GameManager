# GameManager

This project has been developed using Visual Studio Community 2015.

Building from sources we obtains:

GameManager.dll <br />
MyChess.dll <br />
MyTicTacToe.dll <br />
UciClient.exe <br />

In the Zrf folder are the two Zillions of games project: MyChess and MyTicTacToe. <br />
You have to copy GameManager.dll and MyChess.dll in MyChess folder <br />
and GameManager.dll and MyTicTacToe.dll in the MyTicTacToe folder. <br />
<br />
You can launch MyChess or MyTicTacToe clicking on the .zrf files <br />
or running Zillions of games and loading the .zrf file. <br />

UciClient.exe shares the same engine with MyChess and <br />
is a UCI engine that doesn't need any dll. <br />
<br />
<br />
Update: <br />
<br />
Now it's possible cross-compiling having Linux as host (Ubuntu for example) <br />
There are two scripts: <br />

GameManager/CrossCompilingDll.sh creates <br />
GameManager/GameManager/GameManager.dll <br />
GameManager/MyChess/MyChess.dll <br />

GameManager/UciClient/CrossCompiling.sh creates <br />
UciClient/UciClient.exe <br />



