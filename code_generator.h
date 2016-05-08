using namespace std;
using std::string;
using std::queue;
using std::vector;
using std::unordered_map;

// temprary variable
typedef struct Temp_Var
{
	bool string; // true if this is a string temporary variable
	int address; // codePointer + address = address of the real variable
	queue<int> addresses; // addresses of the temprary variables in the runtime environment
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
		int value; // value for int expressions
		unordered_map<string, Temp_Var> tempTable; // temporary variables table
		void printRuntimeEnvironment(); // prints the runtime environment out
		void addStrings(unordered_map<string, int>&); // adds string literals to runtime environment
		void addTemps(AST_Node&); // adds temporary variables to the temp table
		void generateCode(AST_Node&, unordered_map<string, int>&); // generates the code
		void cpPP(); // increments code pointer
		void addTemp(AST_Node&, int); // adds a temporary address to the temp table
		void replaceTemps(); // replaces temporary variables with memory addresses
};

// constructor
Code_Generator::Code_Generator(AST_Node& AST, unordered_map<string, int>& stringsMap, bool v)
{
	// variable initialization
	verbose = v; // set verbose to on or off
	numErrors = 0; // start with no errors
	numWarn = 0; // start with no warnings
	codePointer = 0; // start code at first byte
	value = 0;
	stopPointer = 255; // stop at byte 255 to start - this will change as string literals are added
	// fill the runtime environment with 00s
	for(int i = 0; i < 256; ++i)
		runtime_environment[i] = 0;
	
	// add all string literals to the runtime environment
	addStrings(stringsMap);
	
	// add the necessary temporary variables
	addTemps(AST);
	
	// generate the code
	generateCode(AST, stringsMap);
	
	// replace temporary variable with their memory addresses
	replaceTemps();
	
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

// function to add a temporary variable to the temporary variable table
// var		: the AST node containnig the temporary variable
// address	: the address of the temporary variable
void Code_Generator::addTemp(AST_Node& var, int address)
{
	stringstream keyStream;
	keyStream << var.name.at(1) << "@" << var.scope << "-" << var.subscope;
	string key = keyStream.str();
	Temp_Var& temp = tempTable.at(key);
	temp.addresses.push(address);
}

// function to replace temporary variables
void Code_Generator::replaceTemps()
{
	for ( auto it = tempTable.begin(); it != tempTable.end(); ++it )
	{
		Temp_Var& temp = it->second;
		while(!temp.addresses.empty()) // for each temporary variable address
		{
			runtime_environment[temp.addresses.front()] = codePointer; // set temp variable to memory address that holds the real variable
			temp.addresses.pop();
		}
		cpPP(); // increment the code pointer
	}
}

// function to increment the code pointer and report out of memory errors
void Code_Generator::cpPP()
{
	++codePointer; // increment the code pointer
	if(codePointer > stopPointer) // if we ran out of memory
	{
		codePointer = stopPointer; // prevent trying to access unreachable memory
		if(numErrors < 1) // if we haven't yet reported this, do so
		{
			cout << "[ERROR]" << ": (OOM) " << "The runtime environment is out of memory. Please limit your program to 256 bytes." << endl;
			++numErrors; // increment the number of errors
		}
	}
}

// function to generate the byte code
// ast			: the abstract syntax tree being used
// stringsMap	: map of strings in memory
void Code_Generator::generateCode(AST_Node& ast, unordered_map<string, int>& stringsMap)
{
	// variables
	string name = ast.name;
	string type = ast.type;
	vector<AST_Node>& children = *ast.children;
	
	if(name == "<Block>")
	{
		// recurse on child nodes
		for(vector<AST_Node>::iterator it=children.begin(); it != children.end(); ++it) // for each child node
			generateCode(*it, stringsMap); // recurse
	}
	else if(name == "<VarDecl>")
	{
		AST_Node& var = children.at(1); // the variable
		runtime_environment[codePointer] = 169;
		cpPP(); // increment code pointer
		// set the memoy address of uninitialzed variables to 0 if not a string (reference type), or the last byte for strings (empty string)
		if(var.type == "string") runtime_environment[codePointer] = 255;
		else runtime_environment[codePointer] = 0;
		cpPP();
		runtime_environment[codePointer] = 141;
		cpPP();
		runtime_environment[codePointer] = 0;
		addTemp(var, codePointer); // temp var
		cpPP();
		runtime_environment[codePointer] = 0;
		cpPP();
		return;
	}
	else if(name == "<AssignmentStatement>")
	{
		if(type == "int")
		{
			
		}
		else if(type == "string")
		{
			AST_Node& var = children.at(0); // the variable
			string key = children.at(1).name.substr(2, children.at(1).name.length()-4);
			int addressOfString = stringsMap.at(key); // get the memory address of the string
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = addressOfString;
			cpPP();
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = 0;
			addTemp(var, codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
		}
		else if(type == "boolean")
		{
			
		}
	}
	else if(name == "<PrintStatement>")
	{
		runtime_environment[codePointer] = 0;
		cpPP();
	}
	
	else if(name == "<+>")
	{
		if(children.at(1).name.at(1) > 96 && children.at(1).name.at(1) < 123) // seocnd child is id and not <+>
		{
			if(value > 255) // passed max int value
			{
				cout << "[WARN]Line " << ast.lineNum << ": " << "The maximum value of an integer is 255. Compilation will continue with the max." << endl;
				++numWarn;
				value = 255;
			}
			// load accumulator with the constant value and add the variable's value to it
			// do not store the accumulator in memory
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = value;
			cpPP();
			runtime_environment[codePointer] = 109; // 6d
			cpPP();
			runtime_environment[codePointer] = 0;
			addTemp(children.at(1), codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			value = 0; // reset value
		}
		else if(children.at(1).name.at(1) > 47 && children.at(1).name.at(1) < 58) // seocnd child is a digit and not <+>
		{
			int num = children.at(1).name.at(1) - 48;
			value += num; // add to the value
			if(value > 255) // passed max int value
			{
				cout << "[WARN]Line " << ast.lineNum << ": " << "The maximum value of an integer is 255. Compilation will continue with the max." << endl;
				++numWarn;
				value = 255;
			}
			// load accumulator with the constant value
			// do not store the accumulator in memory
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = value;
			cpPP();
		}
	}
	else if(name.length() == 3 && name.at(1) > 47 && name.at(1) < 58) // [0-9]
	{
		int num = name.at(1) - 48;
		value += num; // add to the value
		return;
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
	// add in the string literals "true" and "false" for booleans (if they don't already exist)
	stringsMap.emplace("true", 0);
	stringsMap.emplace("false", 0);
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
		int hex1 = hex / 16; // the first of the 2 hex digits that make up a byte
		int hex2 = hex % 16; // the second of the 2 hex digits that make up a byte
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
		cout << setw(2) << right << hexDigit1 << hexDigit2 << "| ";
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
