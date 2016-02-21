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
	
	Lexer lex(source); // run the lexer by constructing it
	vector<Token> vec = lex.tokVec; // retreive the token vector
	
	vector<Token>::iterator it;
	for(it = vec.begin(); it != vec.end(); ++it)
		cout << it->name << "[" << it->lineNum << "]" << endl;
	cout << lex.numErrors << "errors found!" << endl;
	
	// catch lexical errors
	if(lex.numErrors > 0)
	{
		// report errors here
		return 1; // exit with errors
	}
	
	////////// PARSE ////////////////////////////////////////////////
	
	return 0; // exit successful
}