# Compilers
Marist College CMPT 432 Project Repository

HOW TO USE:

These steps assume you have already pulled all files from my Github repository and are storing them in the same folder.

TO COMPILE THE COMPILER (Assuming GCC Compiler):

> Open command prompt.
> Type "cd <path the the folder containing these files>" and hit ENTER.
> Type "g++ compiler.cpp -o compiler" and hit ENTER. 
	This will compile the compiler.cpp source file into an executable named compiler.exe.

TO RUN THE COMPILER:

Note: This assumes you already have a working compiler.exe. If not, please follow the instructions TO COMPILE above.

> Open command prompt.
> Type "cd <path the the folder containing these files>" and hit ENTER.
> Type "compiler <full name of source program file to compile>". 
	It is important you include the source program file. 
	Below are the names of all test files included in this repository, but you could of course create your own. 
	If you are keeping source files elsewhere, the entire filepath must be the program argument.
	
	OUTPUT: Upon successful compilation, a program will be output to a text file named ORIGINAL_TEXT_FILE.txt_#.txt,
			where # = the program number associated with the program in the original source text file.
			This text file contains the entire program in hex, which can be directly copy->pasted into SvegOS, or your
			choice of a 6502a supporting platform.

TEST FILES:

Test files I used for each project are found in the "Project X Files" folders, where X = 1, 2, or 3.
