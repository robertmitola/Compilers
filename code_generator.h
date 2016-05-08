using namespace std;
using std::string;
using std::queue;
using std::vector;
using std::unordered_map;

// temprary variable
typedef struct Temp_Var
{
	int address; // codePointer + address = address of the real variable
	queue<int*> addresses; // addresses of the temprary variables in the runtime environment
} Temp_Var;

// jump variable
typedef struct Jump_Var
{
	int distance; // distance to jump
	queue<int*> addresses; // addresses of the temporary jumps in the runtime environment
} Jump_Var;

class Code_Generator
{
	// public class access
	public:
		Code_Generator(AST_Node&, unordered_map<string, int>&, bool); // constructor
		int numErrors; // number of errors
		int numWarn; // number of warnings
		int runtime_environment [256]; // the runtime environment array
	// private class access
	private:
		bool verbose; // verbose output
		int codePointer; // points to where code goes in the runtime environment
		int stopPointer; // points to where code should stop in the runtime environment
		unordered_map<string, Temp_Var> tempTable; // temporary variables table
		void printRuntimeEnvironment(); // prints the runtime environment out
		void addStrings(unordered_map<string, int>&); // adds string literals to runtime environment
		void addTemps(AST_Node&); // adds temporary variables to the temp table
};

// constructor
Code_Generator::Code_Generator(AST_Node& AST, unordered_map<string, int>& stringsMap, bool v)
{
	// variable initialization
	verbose = v; // set verbose to on or off
	numErrors = 0; // start with no errors
	numWarn = 0; // start with no warnings
	codePointer = 0; // start code at first byte
	stopPointer = 255; // stop at byte 255 to start - this will change as string literals are added
	// fill the runtime environment with 00s
	for(int i = 0; i < 256; ++i)
		runtime_environment[i] = 0;
	
	// add all string literals to the runtime environment
	addStrings(stringsMap);
	
	// add the necessary temporary variables
	addTemps(AST);
	
	// verbose mode reporting
	if(verbose)
	{
		// print the runtime environment
		cout <<
			"______________________________________________________________________" << endl <<
			setw(25) << left << "" << "RUNTIME  ENVIRONMENT" << setw(25) << right << "" << endl <<
			"______________________________________________________________________" << endl;
		printRuntimeEnvironment();
		cout << "___|__________________________________________________________________" << endl;
	}
}

// function to add all necessary temporary variables to the temporary variable table
// ast	: the abstract syntax tree to get the variables from
void Code_Generator::addTemps(AST_Node& ast)
{
	vector<AST_Node>& children = *ast.children; // child nodes
	if(ast.name.length() == 3 && ast.name.at(1) > 96 && ast.name.at(1) < 123) // if this is an id
	{
		stringstream keyStream;
		keyStream << ast.name.at(1) << "@" << ast.scope << "-" << ast.subscope;
		string key = keyStream.str();
		Temp_Var* tmp = new Temp_Var;
		tempTable.emplace(key, *tmp);
	}
	// recurse on child nodes
	for(vector<AST_Node>::iterator it=children.begin(); it != children.end(); ++it) // for each child node
		addTemps(*it); // recurse
}

// function to add all string literals to the runtime environment and store their memory addresses
// stringsMap	: the map containing all the strings
void Code_Generator::addStrings(unordered_map<string, int>& stringsMap)
{
	// new stringsMap
	unordered_map<string, int> newMap;
	// for each string in the map
	for(auto it = stringsMap.begin(); it != stringsMap.end(); ++it)
	{
		string str = it->first;
		runtime_environment[stopPointer] = 0; // set the end-of-string character
		--stopPointer; // decrement the stopPointer
		// for each character in the string
		for(int i = str.length(); i > 0; --i)
		{
			if(stopPointer <= codePointer) // if we have run out of memory
			{
				// report error and return
				cout << "[ERROR]" << ": (OOM) " << "The runtime environment is out of memory. Please limit your program to 256 bytes." << endl;
				++numErrors;
				return;
			}
			runtime_environment[stopPointer] = str.at(i-1); // set the character in memory
			--stopPointer; // decrement stop pointer
		}
		// add string to the new map with memory address of string
		newMap.emplace(str, stopPointer+1);
	}
	// update stringsMap
	stringsMap = newMap;
}

// function to print the runtime environment out
void Code_Generator::printRuntimeEnvironment()
{
	for(int i = 0; i < 32; ++i)
	{
		int hex = i*8; // points to corrent row
		cout << setw(3) << right << hex << "| ";
		for(int j = 0; j < 8; ++j)
		{
			int pointer = hex + j; // points to current element of array
			int element = runtime_environment[pointer];
			int hex1 = element / 16; // the first of the 2 hex digits that make up a byte
			int hex2 = element % 16; // the second of the 2 hex digits that make up a byte
			char hexDigit1 = 0;
			char hexDigit2 = 0;
			if(hex1 > 9)
				hexDigit1 = 55 + hex1; // capital letters in ASCII
			else
				hexDigit1 = 48 + hex1; // numbers in ASCII
			if(hex2 > 9)
				hexDigit2 = 55 + hex2; // capital letters in ASCII
			else
				hexDigit2 = 48 + hex2; // numbers in ASCII
			cout << "[" << hexDigit1 << hexDigit2 << "] ";
		}
		cout << endl;
	}
}
