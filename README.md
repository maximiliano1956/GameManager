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
<br />
In Zrf/MyChess folder are also: <br />
GameManager.ini  :  in that file you can enable logging and enable the use of a book of moves <br />
performance.bin  :  the book file <br />
When the book is enabled the analysis mode (UCI go infinite) doesn't work correctly <br />
because if the move searched is marked best move in the book the search stops however <br />
<br />
In Zrf/MyTicTacToe folder are also: <br />
GameManager.ini  :  in that file you can enable logging

UciClient.exe shares the same engine with MyChess and <br />
is a UCI engine that doesn't need any dll. <br />
<br />
<br />
Update: <br />
<br />
Now it's possible cross-compiling having Linux as host using mingw32 <br />
There are two scripts: <br />

GameManager/CrossCompilingDll.sh creates <br />
GameManager/GameManager/GameManager.dll <br />
GameManager/MyChess/MyChess.dll <br />

GameManager/UciClient/CrossCompiling.sh creates <br />
UciClient/UciClient.exe <br />

<br />
Now it's also possibile to compile in Linux:
<br />

The script GameManager/UciClient/Compiling.sh creates <br />
UciClient/UciClient <br />


<br />
Another update: <br />
Now the default evaluation is NNUE! <br />
The name of the net must be "net.nnue" <br />
To switch to the old built-in evaluation use the -UseNNUE=0 line option  <br />
or use UseNNUE uci option <br />
WARNING:  THIS VERSION ACTUALLY DOESN'T COMPILE ANYMORE IN WINDOWS!! (Visual Studio)   <br />
          TO CREATE A WINDOWS VERSION YOU HAVE TO CROSSCOMPILE IN LINUX!   <br />
<br />


