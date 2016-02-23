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

int main()
{
	string source = "test.txt";
	
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
		cout << endl << "[" << lex.numErrors << " lexical errors found.]" << endl << endl;
	
	// catch lexical errors
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
		
	// indicate completion lexical analysis
	cout << endl << "Lexical Analysis complete!" << endl;
	
	////////// PARSE ////////////////////////////////////////////////
	Parser parser(lex.tokQue); // run the Parser by constructing one
	
	
	return 0; // exit successful
}