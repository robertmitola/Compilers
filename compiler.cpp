/*
ROBERT MITOLA
ALAN LABOUSEUR
MARIST COLLEGE
CMPT 432 - DESIGN OF COMPILERS
25 FEBRUARY 2016
*/

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <iomanip>
#include <queue>

#include "lexer.h" 	// The Lexer
#include "parser.h"	// The Parser

using namespace std;
using std::string;
using std::queue;
using std::setw;
using std::cout;
/*
// C++ implementation of PHP's explode
// for splitting the source file at every $ so one can compile multiple programs
// source	: the source string 
// returns	: a queue of separate programs (strings) to compile
queue<string> explode(string const & source)
{
	queue<string> programs;
	istringstream iss(source);
	for(string prog; getline(iss, prog, '$'))
		programs.push(move(prog));
	return programs;
}
*/
int main(int argc, char *argv[])
{
	////////// SETUP ////////////////////////////////////////////////
	
	string fileName; // variable to store the source program filepath
	ifstream sourceFile; // variable to store the filestream
	bool verbose = false; // true if verbose output should occur
	if(argc > 1) // if the user entered a program filepath for compiling
	{
		fileName = argv[1]; // set the source variable to that program filepath
		if(argc > 2)
		{
			string arg = argv[2];
			if(arg == "verbose") verbose = true;
		}
	}
	else // if s/he didn't...
	{
		cout << endl << "The filepath to your program must be entered as an argument when running this parser." << endl;
		return 1; // exit with errors
	}
	
	cout << endl << "Opening source program file..." << endl;
	sourceFile.open(fileName.c_str()); // open the given file
	if(sourceFile.fail()) // if the file cannot be opened
	{
		cout << "Error opening source program file. Please make sure the specified path to the file is correct." << endl; // print appropriate error message
		return 1; // exit with error
	}
	
	// read source file into a string
	string source((istreambuf_iterator<char>(sourceFile)), (istreambuf_iterator<char>()));
	
	// split the source into multiple programs (if necessary)
	regex eof("([$]|[^$]+)"); // match anything and EOF and anything after
	regex_iterator<string::iterator> regIt(source.begin(), source.end(), eof);
	regex_iterator<string::iterator> regEnd;
	string toAdd = "";
	bool leftover = false;
	queue<string> programs; // queue to hold all programs
	while(regIt != regEnd)
	{
		string txt = regIt->str();
		if(txt == "$")
		{
			programs.push(toAdd+"$");
			toAdd = "";
			leftover = false;
		}else{
			toAdd = txt;
			leftover = true;
		}
		++regIt;
	}
	if(leftover) programs.push(toAdd); // add program with forgotten $
	
	// compile all the programs in order
	int progNum = 1; // program number for keeping track of which is being compiled
	while(!programs.empty())
	{
		cout << " ____________________________" << endl << 
		"| COMPILING PROGRAM No. " << setw(5) << progNum << "|" << endl <<
		"+_____________________________________________________________________"<< endl;
	
		////////// LEX //////////////////////////////////////////////////
		cout << "Performing Lexical Analysis..." << endl << endl;
		Lexer lex(programs.front()); // run the lexer by constructing one
		queue<Token> que = lex.tokQue; // retreive the token queue
	
		// report lexical errors here
		cout << endl << "[" << lex.numErrors << " lexical error(s) found.]"
		<< " [" << lex.numWarnings << " lexical warning(s) found.]" << endl << endl;
	
		// exit if lexical errors were found
		if(lex.numErrors > 0)
		{
			// skip to next program
			programs.pop(); // on to the next program
			continue; 
		}
	
		// print out the token information if verbose is on
		if(verbose)
		{
			cout <<
				"______________________________________________________________________" << endl <<
				setw(30) << left << "" << "TOKEN LIST" << setw(30) << right << "" << endl <<
				"______________________________________________________________________" << endl;
			while(!que.empty())
			{
				cout << left <<
					"[NAME: " << setw(15) << que.front().name << "]" << 
					"[VALUE: " << setw(10) << que.front().value << "]" << 
					"[LINE: " << setw(20) << que.front().lineNum << "]" << endl;
				que.pop();
			}
			cout << "______________________________________________________________________" << endl;
		}
		
		// indicate completion of lexical analysis
		cout << endl << "Lexical Analysis complete!" << endl;
	
		////////// PARSE ////////////////////////////////////////////////
		cout << endl << "Performing Parsing..." << endl << endl;
		Parser parse(lex.tokQue, verbose); // run the Parser by constructing one
	
		// report parser errors here
		cout << endl << "[" << parse.numErrors << " parse error(s) found.]" << endl;
	
		// exit if parser errors were found
		if(parse.numErrors > 0)
		{
			// skip to next program
			programs.pop(); // on to the next program
			continue; 
		}
	
		// indicate completion of parsing
		cout << endl << "Parsing complete!" << endl;
	
		programs.pop(); // on to the next program
		++progNum; // so increment the program number
	}
	return 0; // exit successful
}