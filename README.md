# Serial Port Communication

## Overview

This is a complete implementation of an AT-Command based serial port communication system using socat. The server handles a linked list of registers, 
depending on the client's commands.  

## Compile Instructions 

To compile the program, follow these steps:
	1. cmake -B <executables_directory> 
	2. cd <executables_directory>
	3. make

In <executables_directory> feel free to add any name you like. A new directory with the desired name will be created. If you want all the files to 
be generated in the source code directory, write "." instead of a directory name and skip step 2. 

I personally compiled the programs in a new directory named "executables".

## Run Instructions 

After the compilation, in order to run the program, three terminals are needed. The first is for the socat connection, while the other two are for
the server and client applications respectively.

Run these commands:
	First terminal: socat -d -d pty,link=<file1> pty,link=<file2>
	Second terminal: ./server <file1>
	Third terminal: ./client <file2> 

Then go to the client terminal to send commands to the server from there. 

## User avaiable actions 

In the client program, the actions available to the user are the following: 
	1. 'AT+REG' (including reg number) command to print the selected register's value.
	2. 'AT+REG=?' command to print the selected register's bounds of accepted values.
	3. 'AT+REG=<int>' command, where <int> any given integer, to replace selected register's value with <int>, if within accepted bounds.
	4. 'insert+<int>+<bounds>' to insert a new register to the list with value <int> and <bounds> (as string).
	5. 'help' to print the available AT-Commands.
	6. 'quit' to send a termination request to the server in order for both programs to terminate. 

## Other notes

1. The code is written using the GNU coding style.
2. Since I hadn't used tty-like terminals and the termios.h library, in order to establish the communication, I got help from these two links:

https://gist.github.com/wdalmut/7480422
https://gist.github.com/wdalmut/7480422

If you have any questions, feel free to ask me.  
