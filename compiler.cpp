#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "lexer.h"

using namespace std;
using std::string;
using std::vector;

int main()
{
	string source = "test.txt";
	
	////////// LEX //////////////////////////////////////////////////
	
	cout << "Opening source program file..." << endl;
	
	Lexer lex(source); // run the lexer by constructing it
	vector<Token> vec = lex.tokVec; // retreive the token vector
	
	if(lex.numErrors == -1) // file stream error
	{
		cout << "Error opening source program file. Please make sure the specified path to the file is correct." << endl; // print appropriate error message
		return 1; // exit with errors
	}
	
	// catch lexical errors
	if(lex.numErrors > 0)
	{
		// report errors here
		cout << "[" << lex.numErrors << " errors found.]" << endl;
		
		return 1; // exit with errors
	}
	
	vector<Token>::iterator it;
	for(it = vec.begin(); it != vec.end(); ++it)
		cout << "[NAME: "  << it->name << "] [VALUE: " << it->value << "] [LINE: " << it->lineNum << "]" << endl;
	
	////////// PARSE ////////////////////////////////////////////////
	
	return 0; // exit successful
}