using namespace std;
using std::string;
using std::vector;

// the token structure
struct Token
{
	string name;	// the name of this token
	int lineNum;	// the line number this token is on
};

// the Lexer class object definition
class Lexer
{
	// public class access
	public:
		Lexer(string); // constructor
		vector<Token> tokVec; // a vector to hold all the resulting tokens
		int numErrors; // the number of errors encountered while scanning
	// private class access
	private:
		void addToken(vector<Token> &, string &, int lineNum, int &);
};

// the Lexer constructor
// sourceFile	: the source program file the lexer should read from
Lexer::Lexer(string sourceFile)
{
	// file input variables
	ifstream source; // input file stream to read from the source file
	string input; // stores what is being fed into this program from a file stream
	char next; // the next character in the input stream
	
	// other important variables
	int lineNum = 1; // the line number the parser is currently on
	int i = 0; // holder for the number to correspond with each valid character for matrix traversal
	int state = 0; // the current DFA state
	string tokName = ""; // variable to hold the name of tokens found while parsing
	numErrors = 0; // start with no errors
	
	// transition table for the DFA
	int transitionTable [31][45] = // characters x states
	{
		{1,24,1,1,1,15,1,1,3,1,1,1,1,1,1,5,1,1,10,18,1,1,20,1,1,1,2,2,2,2,2,2,2,2,2,2,30,30,30,30,30,30,30,4,9}, // State : 0
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 1
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 2
		{0,0,0,0,0,30,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 3
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,0}, // State : 4
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 5
		{0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 6
		{0,0,0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 7
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 8
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,0}, // State : 9
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 10
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 11
		{0,0,0,0,0,0,0,0,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 12
		{0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 13
		{0,0,0,0,0,0,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 14
		{16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 15
		{0,0,0,0,0,0,0,0,0,0,0,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 16
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 17
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 18
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 19
		{0,0,0,0,0,0,0,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 20
		{0,0,0,0,0,0,0,0,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 21
		{0,0,0,0,0,0,0,0,0,0,0,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 22
		{0,0,0,0,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 23
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 24
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 25
		{0,0,0,0,0,0,0,0,0,0,0,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 26
		{0,0,0,0,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 27
		{29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 28
		{0,0,0,0,0,0,0,0,0,0,0,0,0,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // State : 29
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // State : 30
	};
	
	source.open(sourceFile.c_str()); // open the given file
	if(source.fail()) // if the file cannot be opened
	{
		cout << "Error opening source program file. Please make sure the specified path to the file is correct." << endl; // print appropriate error message
		return; // exit with error
	}
	
	if(source.is_open())
	{
		bool loop = source.get(next); // store the next character in next and set loop to false if this is the end of file
		bool readingCharList = false; // true if the lexer should be reading a CharList
		while(loop)
		{
			bool error = false; // becomes true if a character found is not a part of the grammar
			switch(next)
			{
				case 'a':
					i = 0;
					break;
				case 'b':
					i = 1;
					break;
				case 'c':
					i = 2;
					break;
				case 'd':
					i = 3;
					break;
				case 'e':
					i = 4;
					break;
				case 'f':
					i = 5;
					break;
				case 'g':
					i = 6;
					break;
				case 'h':
					i = 7;
					break;
				case 'i':
					i = 8;
					break;
				case 'j':
					i = 9;
					break;
				case 'k':
					i = 10;
					break;
				case 'l':
					i = 11;
					break;
				case 'm':
					i = 12;
					break;
				case 'n':
					i = 13;
					break;
				case 'o':
					i = 14;
					break;
				case 'p':
					i = 15;
					break;
				case 'q':
					i = 16;
					break;
				case 'r':
					i = 17;
					break;
				case 's':
					i = 18;
					break;
				case 't':
					i = 19;
					break;
				case 'u':
					i = 20;
					break;
				case 'v':
					i = 21;
					break;
				case 'w':
					i = 22;
					break;
				case 'x':
					i = 23;
					break;
				case 'y':
					i = 24;
					break;
				case 'z':
					i = 25;
					break;
				case '0':
					i = 26;
					break;
				case '1':
					i = 27;
					break;
				case '2':
					i = 28;
					break;
				case '3':
					i = 29;
					break;
				case '4':
					i = 30;
					break;
				case '5':
					i = 31;
					break;
				case '6':
					i = 32;
					break;
				case '7':
					i = 33;
					break;
				case '8':
					i = 34;
					break;
				case '9':
					i = 35;
					break;
				case '+':
					i = 36;
					break;
				case '{':
					i = 37;
					break;
				case '}':
					i = 38;
					break;
				case '(':
					i = 39;
					break;
				case ')':
					i = 40;
					break;
				case '"':
					readingCharList = !readingCharList; // either start or end quotes
					i = 41;
					break;
				case '$':
					i = 42;
					break;
				case '=':
					i = 43;
					break;
				case '!':
					i = 44;
					break;
				case '\n':
					readingCharList = false; // the lexer should NOT include \n in a CharList
					loop = source.get(next); // go on to the next character
					lineNum++; // incremenet the current line number
					// state = 0; // ROB												ROB ROB 			ROB !!!!!!!!!!!!!!!!
					// ^^ you'll have to find some other way to handle this
					continue; // jump to the end of this loop iteration
					break;
				case ' ':
					if(readingCharList) // is this space part of a CharList?
					{
						cout << "Definite space character" << endl;
						Token tok = Token{"space", lineNum}; // create a new token
						tokVec.push_back(tok); // append token to the token vector
					}
					loop = source.get(next); // go on to the next character
					continue; // jump to the end of this loop iteration
					break;
				default:
					error = true; // this character is not found in the grammar
					break;
			}
			
			int fromState = state; // for testing - delete
			if(error)
				state = 0;
			else
				state = transitionTable[state][i];
			cout << "PREV STATE: [" << fromState << "] NEW STATE: [" << state << "] CHAR: [" << next << "]" << endl;
			tokName += next;
			
			switch(state)
			{
				case 0: // we arrive here either due to the start of a new token or an error being found
					int max; // number of characters to loop through in tokName
					if(tokName.size() == 1) 
					{
						max = 1; // iterate once if there is only one character in tokName
						loop = source.get(next); // go on to the next character
					}
					else 
						max = tokName.size()-1; // iterate through this loop for every character in tokName except the last
					for(string::size_type j = 0; j < max; ++j)
					{
						char& c = tokName[j];
						if((c >= 'a' && c <= 'z') || c == '=') // if the character c is a lowercase letter a-z OR '='
						{
							cout << "Definite id " << c << endl;
							string s; // string to hold the single character
							stringstream ss; // string stream for converting single characters into strings
							ss << c; // send character c to string stream ss 
							ss >> s; // stream string into string s
							Token tok = Token{s, lineNum}; // create a new token
							tokVec.push_back(tok); // append token to the token vector
						}
						else
						{
							cout << "ERROR: " << c << " is not a valid token." << endl;
							++numErrors; // increment the number of errors found
						}
					}
					tokName = ""; // reset the token name
					continue; // jump to the end of this loop iteration
					break;
				case 1: // this state means we've found an id
					cout << "Definite id " << tokName << endl;
					addToken(tokVec, tokName, lineNum, state);
					break;
				case 2: // this state means we've found an integer
					cout << "Definite integer " << tokName << endl;
					addToken(tokVec, tokName, lineNum, state);
					break;
				case 30: // this state means we've found a reserve word (e.g. print)
					cout << "Definite reserved word " << tokName << endl;
					addToken(tokVec, tokName, lineNum, state);
					break;
				default:
					// do nothing - we are not in an accepting state
					break;
			}
			loop = source.get(next); // go on to the next character
		}
	}
	source.close(); // close the file input streams
	// return 0; // return successful
}

// function to add a token to the token vector
// tokVec	: the token vector, to add tokens to
// tokName	: the name of the token, passed by reference for manipulation
// lineNum	: the current line number we are scanning
// state	: the current state, to be set to 0
void Lexer::addToken(vector<Token> & tokVec, string & tokName, int lineNum, int & state)
{
	Token tok = Token{tokName, lineNum};
	tokVec.push_back(tok); // push the token to the back of the token vector
	tokName = ""; // reset the token name
	state = 0; // reset the state
}