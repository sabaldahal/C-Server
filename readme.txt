Author: Sabal Dahal
sabal.dahal@outlook.com


COMPILE INSTRUCTIONS:

The Server Code is written in C++ and should be compiled on Windows machine. (Currently, it is only supported on Windows. Will make necessary adjustments in the future to support more platforms.)

During compilation add the argument "-lws2_32". This is to compile the program correctly. Alternatively, if compiling from VSCode, include this argument in the tasks.json file.

PROGRAM DETAILS:

webserver.cpp is the main file that contains main() function. It is the entry point for the program.
All functions are self explanatory which is clearly reflected by their names.

Client Code is written in Java and the compilation is very simple. Compile with default arguments.

Old Issues:
- Linker Issue: Initially, I could not find what the issue with the compilation error was, but later found that it was to do with the linkers. Adding the "-lws2_32" argument during compilation fixed that issue.


USABILITY INSTRUCTIONS:
Run the server
For client:
- Run client code
- Connect to the server by using:
	CONNECT localhost 6789
- Before sending any of the commands, send client's username to the server. This can be sent using the command:
	CREDENTIAL username

Note: For every command sent by the client, the server will respond back with a message.

Note For HTTP requests:
place static files in the same directory under the folder "static".
	
	
