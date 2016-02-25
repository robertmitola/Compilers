#include <iostream>
#include <fstream>
#include <string>
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

int main(int argc, char *argv[])
{
	////////// SETUP ////////////////////////////////////////////////
	
	string source; // variable to store the source program filepath
	if(argc > 1) // if the user entered a program filepath for compiling
	{
		source = argv[1]; // set the source variable to that program filepath
	}
	else // if s/he didn't...
	{
		cout << endl << "The filepath to your program must be entered as an argument when running this parser." << endl;
		return 1; // exit with errors
	}
	
	////////// LEX //////////////////////////////////////////////////
	
	cout << endl << "Opening source program file..." << endl << endl;
	
	Lexer lex(source); // run the lexer by constructing one
	queue<Token> que = lex.tokQue; // retreive the token queue
	
	if(lex.numErrors == -1) // file stream error
	{
		cout << "Error opening source program file. Please make sure the specified path to the file is correct." << endl; // print appropriate error message
		return 1; // exit with errors
	}
	
	// report lexical errors here
	cout << endl << "[" << lex.numErrors << " lexical error(s) found.]"
	<< " [" << lex.numWarnings << " lexical warning(s) found.]" << endl << endl;
	
	// exit if lexical errors were found
	if(lex.numErrors > 0)
	{
		return 1; // exit with errors
	}
	
	// print out the token information
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
		
	// indicate completion of lexical analysis
	cout << endl << "Lexical Analysis complete!" << endl;
	
	////////// PARSE ////////////////////////////////////////////////
	Parser parse(lex.tokQue); // run the Parser by constructing one
	
	// report parser errors here
	cout << endl << "[" << parse.numErrors << " parse error(s) found.]" << endl;
	
	// exit if parser errors were found
	if(parse.numErrors > 0)
	{
		return 1; // exit with errors
	}
	
	// indicate completion of parsing
	cout << endl << "Parsing complete!" << endl;
	
	return 0; // exit successful
}