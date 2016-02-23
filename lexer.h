using namespace std;
using std::string;
using std::queue;

// the Token structure
struct Token
{
	string value;	// the value of this token
	string name;	// the name of this token
	int lineNum;	// the line number this token is on
};

// the Lexer class object definition
class Lexer
{
	// public class access
	public:
		Lexer(string); // constructor
		queue<Token> tokQue; // a quetor to hold all the resulting tokens
		int numErrors; // the number of errors encountered while scanning
	// private class access
	private:
		void addToken(queue<Token>&, string&, string, int, int&);
		string getTokenName(string);
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
	string tokVal = ""; // variable to hold the value of tokens found while parsing
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
		numErrors = -1; // specifies there is a file stream error
		return; // exit with error
	}
	
	cout << "Performing Lexical Analysis..." << endl;
	
	if(source.is_open())
	{
		// store the next character in next and set loop to false if this is the end of file
		// static casting to correct Microsoft VS 2013 compiler istream to bool conversion error
		bool loop = static_cast<bool> (source.get(next)); 
		bool readingCharList = false; // true if the lexer should be reading a CharList
		while(loop)
		{
			bool error = false; // becomes true if a character found is not a part of the grammar
			bool newline = false; // becomes true if a character found is the newline character
			bool spaceChar = false; // becomes true if a space is found in a CharList
			// get the int counterpart to the current scanned char for state table traversal
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
					newline = true;
					break;
				case '\r':
					readingCharList = false; // the lexer should NOT include \n in a CharList
					newline = true;
					break;
				case ' ':
					if(!readingCharList) // is this space not part of a CharList?
					{
						loop = static_cast<bool> (source.get(next)); // go on to the next character
						continue; // jump to the end of this loop iteration
					}
					// this space IS part of a CharList
					spaceChar = true;
					break;
				default:
					error = true; // this character is not found in the grammar
					break;
			}
			
			int fromState = state; // for testing
			if(error || newline || spaceChar)
				state = 0;
			else
				state = transitionTable[state][i];
			tokVal += next; // add the next character to the token value
			
			// cout << "PREV STATE: [" << fromState << "] NEW STATE: [" << state << "] CHAR: [" << next << "]" << endl;
			
			switch(state)
			{
				case 0: // we arrive here either due to the start of a new token or an error being found
					int max; // number of characters to loop through in tokVal
					if(tokVal.size() == 1) 
					{
						max = 1; // iterate once if there is only one character in tokVal
						loop = static_cast<bool> (source.get(next)); // go on to the next character
					}
					else 
						max = tokVal.size()-1; // iterate through this loop for every character in tokVal except the last
					for(string::size_type j = 0; j < max; ++j)
					{
						char& c = tokVal[j];
						if((c >= 'a' && c <= 'z') || c == '=' || c == ' ') // if the character c is a lowercase letter a-z OR '=' OR ' ' (space char)
						{
							// cout << "Definite " << c << endl;
							string s; // string to hold the single character
							stringstream ss; // string stream for converting single characters into strings
							ss << c; // send character c to string stream ss 
							ss >> s; // stream string into string s
							string name; // name of the token
							if(c == '=') name = "T_ASSIGN";
							else if(c == ' ') name = "T_SPACE";
							else name = "T_ID";
							Token tok = Token{s, name, lineNum}; // create a new token
							tokQue.push(tok); // append token to the token quetor
						}
						else if(c == '\n' || c == '\r') // new line 
						{
							lineNum++; // incremenet the current line number
							// also effectively ends token analysis at the newline char
						}					
						else
						{
							cout << "[ERROR]Line " << lineNum << ": " << c << " is not a valid lexeme." << endl;
							++numErrors; // increment the number of errors found
						}
					}
					tokVal = ""; // reset the token name
					continue; // jump to the end of this loop iteration
					break;
				case 1: // this state means we've found an id
					// cout << "Definite id " << tokVal << endl;
					addToken(tokQue, tokVal, "T_ID", lineNum, state);
					break;
				case 2: // this state means we've found an integer
					// cout << "Definite integer " << tokVal << endl;
					addToken(tokQue, tokVal, "T_DIGIT", lineNum, state);
					break;
				case 30: // this state means we've found a reserve word (e.g. print)
					// cout << "Definite reserved word " << tokVal << endl;
					addToken(tokQue, tokVal, getTokenName(tokVal), lineNum, state);
					break;
				default:
					// do nothing - we are not in an accepting state
					break;
			}
			loop = static_cast<bool> (source.get(next)); // go on to the next character
		}
	}
	source.close(); // close the file input streams
}

// function to add a token to the token quetor
// tokQue	: the token quetor, to add tokens to
// tokVal	: the value of the token, passed by reference for manipulation
// tokName	: the name of the token
// lineNum	: the current line number we are scanning
// state	: the current state, to be set to 0
void Lexer::addToken(queue<Token>& tokQue, string& tokVal, string tokName, int lineNum, int& state)
{
	Token tok = Token{tokVal, tokName, lineNum};
	tokQue.push(tok); // push the token to the back of the token quetor
	tokVal = ""; // reset the token name
	state = 0; // reset the state
}

// function to get the corresponding name of a token value if it is not T_ID or T_DIGIT
// value	: the value of the token
// returns	: the name of the token
string Lexer::getTokenName(string val)
{
	// I used if statements here since C++ switch statements cannot handle strings
	if(val == "+")
		return "T_PLUS";
	if(val == "{")
		return "T_OPEN_BRACE";
	if(val == "}")
		return "T_CLOSE_BRACE";
	if(val == "(")
		return "T_OPEN_PAREN";
	if(val == ")")
		return "T_CLOSE_PAREN";
	if(val == "\"")
		return "T_QUOTE";
	if(val == "$")
		return "T_EOF";
	if(val == "==")
		return "T_EQUALS";
	if(val == "!=")
		return "T_NOT_EQUALS";
	if(val == "false")
		return "T_FALSE";
	if(val == "true")
		return "T_TRUE";
	if(val == "while")
		return "T_WHILE";
	if(val == "print")
		return "T_PRINT";
	if(val == "int")
		return "T_INT";
	if(val == "string")
		return "T_STRING";
	if(val == "boolean")
		return "T_BOOLEAN";
	if(val == "if")
		return "T_IF";
	return "UNKNOWN_NAME"; // this should never occur, but is here just in case, and for testing
}