/*
ROBERT MITOLA
ALAN LABOUSEUR
MARIST COLLEGE
CMPT 432 - DESIGN OF COMPILERS
14 APRIL 2016
*/

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sstream>
#include <iomanip>
#include <queue>
#include <unordered_map>
#include <vector>

#include "lexer.h" 	// The Lexer
#include "parser.h"	// The Parser
#include "semantic_analyzer.h" // The Semantic Analyzer
#include "code_generator.h" // The Code Generator

using namespace std;
using std::string;
using std::queue;
using std::setw;
using std::cout;

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
	int progNum = 0; // program number for keeping track of which is being compiled
	while(!programs.empty())
	{
		++progNum; // so increment the program number
		if(progNum > 1) // if not compiling first program
		{
			cout << endl << "PRESS ENTER TO COMPILE NEXT PROGRAM... (Ctrl+C to stop.)" << endl << endl;
			cin.ignore(); // cin to pause program compilation
		}
		
		cout << " _____________________________" << endl << 
		"| COMPILING PROGRAM No. " << setw(5) << progNum << " |" << endl <<
		"+_____________________________+_______________________________________"<< endl;
	
		////////// LEX //////////////////////////////////////////////////
		cout << "Performing Lexical Analysis..." << endl;
		Lexer lex(programs.front()); // run the lexer by constructing one
		queue<Token> que = lex.tokQue; // retreive the token queue
	
		// report lexical errors here
		cout << "[" << lex.numErrors << " lexical error(s) found.]"
		<< " [" << lex.numWarnings << " lexical warning(s) found.]" << endl;
	
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
		cout << "Lexical Analysis complete!" << endl;
	
		////////// PARSE ////////////////////////////////////////////////
		cout << "Performing Parsing..." << endl;
		Parser parse(lex.tokQue, verbose); // run the Parser by constructing one
	
		// report parser errors here
		cout << "[" << parse.numErrors << " parse error(s) found.]" << endl;
	
		// exit if parser errors were found
		if(parse.numErrors > 0)
		{
			// skip to next program
			programs.pop(); // on to the next program
			continue; 
		}
	
		// indicate completion of parsing
		cout << "Parsing complete!" << endl;
		
		////////// SEMANTIC ANALYSIS ///////////////////////////////////
		cout << "Performing Semantic Analysis..." << endl;
		Semantic_Analyzer semantics(parse.CST, verbose);
		
		// report semantic errors here
		cout << "[" << semantics.numErrors << " semantic error(s) found.]"
		<< " [" << semantics.numWarn << " semantic warning(s) found.]" << endl;
	
		// exit if semantic errors were found
		if(semantics.numErrors > 0)
		{
			// skip to next program
			programs.pop(); // on to the next program
			continue; 
		}
		
		// indicate completion of semantic analysis
		cout << "Semantic Analysis complete!" << endl;
		
		////////// CODE GENERATION /////////////////////////////////////
		cout << "Performing Code Generation..." << endl;
		Code_Generator codeGen(semantics.AST, semantics.stringsMap, verbose);
		
		// report code gen errors here
		cout << "[" << codeGen.numErrors << " code generation error(s) found.]"
		<< " [" << codeGen.numWarn << " code generation warning(s) found.]" << endl;
		
		// exit if code gen errors were found
		if(codeGen.numErrors > 0)
		{
			// skip to next program
			programs.pop(); // on to the next program
			continue; 
		}
		
		// indicate completion of code generation
		cout << "Code Generation complete!" << endl;
		
		programs.pop(); // on to the next program
	}
	cout << "End of compilation." << endl;
	return 0; // exit successful
}