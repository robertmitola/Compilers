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

class Code_Generator
{
	// public class access
	public:
		Code_Generator(AST_Node&, unordered_map<string, int>&, bool); // constructor
		int numErrors; // number of errors
		int numWarn; // number of warnings
		int runtime_environment [256]; // the runtime environment array
		string hex;
	// private class access
	private:
		bool verbose; // verbose output
		int codePointer; // points to where code goes in the runtime environment
		int stopPointer; // points to where code should stop in the runtime environment
		int value; // value for int expressions
		unordered_map<string, Temp_Var> tempTable; // temporary variables table
		vector<int> jumps; // alters jump values
		void printRuntimeEnvironment(); // prints the runtime environment out
		void addStrings(unordered_map<string, int>&); // adds string literals to runtime environment
		void addTemps(AST_Node&); // adds temporary variables to the temp table
		void generateCode(AST_Node&, unordered_map<string, int>&); // generates the code
		void cpPP(); // increments code pointer
		void addTemp(AST_Node&, int); // adds a temporary address to the temp table
		void replaceTemps(); // replaces temporary variables with memory addresses
		void create6502aCode(); // function to turn the code into a string
		void hexTrace(); // for testing
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
	// append the 00 byte to signify the EOP
	runtime_environment[codePointer] = 0;
	cpPP();
	
	// replace temporary variable with their memory addresses
	replaceTemps();
	
	// verbose mode reporting
	if(verbose)
	{
		// trace the creation of the 6502a codes
		hexTrace();
		// print the runtime environment
		cout <<
			"______________________________________________________________________" << endl <<
			setw(25) << left << "" << "RUNTIME  ENVIRONMENT" << setw(25) << right << "" << endl <<
			"______________________________________________________________________" << endl;
		printRuntimeEnvironment();
		cout << "___|__________________________________________________________________" << endl;
	}
	
	// push the code to a string for outputting to a text file
	create6502aCode();
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
void Code_Generator::cpPP() // "code pointer plus plus"
{
	++codePointer; // increment the code pointer
	// increment all jump values relevant
	for (vector<int>::iterator it = jumps.begin() ; it != jumps.end(); ++it)
	{
		runtime_environment[*it] = runtime_environment[*it] + 1;
	}
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
		if(children.at(1).name.length() == 3 && children.at(1).name.at(1) > 96 && children.at(1).name.at(1) < 123) // if the assignment is one variable to another
		{
			AST_Node& var1 = children.at(0); // the variable to assign
			AST_Node& var2 = children.at(1); // the variable assigning
			// load variable to give value
			runtime_environment[codePointer] = 173; // ad
			cpPP();
			runtime_environment[codePointer] = 0;
			addTemp(var2, codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			// store variable to get value
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = 0;
			addTemp(var1, codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
		}
		else if(type == "int")
		{
			AST_Node& var = children.at(0); // the variable
			if(children.at(1).name.length() == 3 && children.at(1).name.at(1) > 47 && children.at(1).name.at(1) < 58) // if the assignment is simply a digit
			{
				int num = children.at(1).name.at(1) - 48;
				runtime_environment[codePointer] = 169; // a9
				cpPP();
				runtime_environment[codePointer] = num;
				cpPP();
				runtime_environment[codePointer] = 141; // 8d
				cpPP();
				runtime_environment[codePointer] = 0;
				addTemp(var, codePointer); // temp var
				cpPP();
				runtime_environment[codePointer] = 0;
				cpPP();
			}
			else // assignment had a <+> in it
			{
				generateCode(children.at(1), stringsMap); // recurse on <+>
				// after recursing, the accumulator should contain the correct number to assign
				// so all that needs to be done is to store the accumulator in memory
				runtime_environment[codePointer] = 141; // 8d
				cpPP();
				runtime_environment[codePointer] = 0;
				addTemp(var, codePointer); // temp var
				cpPP();
				runtime_environment[codePointer] = 0;
				cpPP();
			}
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
			AST_Node& var = children.at(0); // the variable
			if(children.at(1).name == "[true]" || children.at(1).name == "[false]") // simply asssign true / false
			{
				int boolean = 0;
				if(children.at(1).name == "[true]") boolean = 1; // set boolean to correct memory value
				// load accumulator with constant
				runtime_environment[codePointer] = 169; // a9
				cpPP();
				runtime_environment[codePointer] = boolean;
				cpPP();
				// store variable
				runtime_environment[codePointer] = 141; // 8d
				cpPP();
				runtime_environment[codePointer] = 0;
				addTemp(var, codePointer); // temp var
				cpPP();
				runtime_environment[codePointer] = 0;
				cpPP();
			}
			else // right hand side of expression is <==> or <!=>
			{
				generateCode(children.at(1), stringsMap); // recurse on <==> or <!=>
				// final value of a nested boolean expression will be stored in the last memory address
				// load accumulator with the memory at this address
				runtime_environment[codePointer] = 173; // ad
				cpPP();
				runtime_environment[codePointer] = codePointer-2; // address of last boolean push to memory
				cpPP();
				runtime_environment[codePointer] = 0;
				// store variable
				runtime_environment[codePointer] = 141; // 8d
				cpPP();
				runtime_environment[codePointer] = 0;
				addTemp(var, codePointer); // temp var
				cpPP();
				runtime_environment[codePointer] = 0;
				cpPP();
			}
		}
	}
	else if(name == "<PrintStatement>")
	{
		AST_Node& rhs = children.at(0);
		if(rhs.name.length() == 3 && rhs.name.at(1) > 47 && rhs.name.at(1) < 58) // right hand digit
		{
			int num = rhs.name.at(1) - 48;
			// load the x register with a constant representing "print value in y register"
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1; 
			cpPP();
			// load y register with a constant to print
			runtime_environment[codePointer] = 160; // a0
			cpPP();
			runtime_environment[codePointer] = num; 
			cpPP();
		}
		else if(rhs.name.length() == 3 && rhs.name.at(1) > 96 && rhs.name.at(1) < 123) // right hand id
		{
			// load the x register with correct constant for printing
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			if(rhs.type == "int" || rhs.type == "boolean") runtime_environment[codePointer] = 1; 
			else if(rhs.type == "string") runtime_environment[codePointer] = 2;
			cpPP();
			// load y register with memory to print
			runtime_environment[codePointer] = 172; // ac
			cpPP();
			runtime_environment[codePointer] = 0; 
			addTemp(rhs, codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0; 
			cpPP();
		}
		else if(rhs.name == "[true]" || rhs.name == "[false]")
		{
			string key = rhs.name.substr(1, rhs.name.length()-2); // true / false
			int addressOfString = stringsMap.at(key); // get the memory address of the string
			// load the x register with a constant representing "print string at address in y register"
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 2; 
			cpPP();
			// load y register with memory address of the string as a constant
			runtime_environment[codePointer] = 160; // a0
			cpPP();
			runtime_environment[codePointer] = addressOfString; 
			cpPP();
		}
		else if(rhs.name == "<+>")
		{
			generateCode(rhs, stringsMap); // recurse on <+>
			// after recursing, the accumulator should contain the correct number to assign
			// so all that needs to be done is to print the accumulator in memory
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer+1; // store accumulator value at next address
			cpPP();
			runtime_environment[codePointer] = 0; // accumulator value will be stored here
			cpPP();
			// load the x register with a constant representing "print value in y register"
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1; 
			cpPP();
			// load y register with memory to print
			runtime_environment[codePointer] = 172; // ac
			cpPP();
			runtime_environment[codePointer] = codePointer-4; // accumulator value should be stored there
			cpPP();
			runtime_environment[codePointer] = 0; 
			cpPP();
		}
		else if(rhs.name == "<==>" || rhs.name == "<!=>")
		{
			generateCode(rhs, stringsMap); // recurse on <==> or <!=>
			// final value of a nested boolean expression will be stored in the last memory address
			// load y with the memory at this address
			runtime_environment[codePointer] = 172; // ac
			cpPP();
			runtime_environment[codePointer] = codePointer-2; // address of last boolean push to memory
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			// load x register with a constant representing "print value in y register"
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1; 
			cpPP();
			cout << "got here" << endl;
		}
		else // string literal
		{
			string key = rhs.name.substr(2, rhs.name.length()-4);
			int addressOfString = stringsMap.at(key); // get the memory address of the string
			// load the x register with a constant representing "print string at address in y register"
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 2; 
			cpPP();
			// load y register with memory address of the string as a constant
			runtime_environment[codePointer] = 160; // a0
			cpPP();
			runtime_environment[codePointer] = addressOfString; 
			cpPP();
		}
		// print using system call
		runtime_environment[codePointer] = 255;
		cpPP();
	}
	else if(name == "<==>" || name == "<!=>")
	{
		AST_Node& left = children.at(0); // left hand side
		int leftBool; // left hand stored bool compare value address
		AST_Node& right = children.at(1); // right hand side
		
		// LEFT HAND SIDE
		
		if(left.name == "<==>" || left.name == "<!=>")
		{
			generateCode(left, stringsMap); // recurse on <==> or <!=>
			leftBool = codePointer-1; // address of last boolean push to memory
		}
		else if(left.name.length() == 3 && left.name.at(1) > 47 && left.name.at(1) < 58) // left hand digit
		{
			int num = left.name.at(1) - 48;
			// load accumulator with constant
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = num;
			cpPP();
			// store accumulator in the unused memory address that is a part of the isntruction
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer + 1;
			cpPP();
			runtime_environment[codePointer] = 0; // value will be stored here
			leftBool = codePointer; // this is where to eventually get the left variable
			cpPP();
		}
		else if(left.name.length() == 3 && left.name.at(1) > 96 && left.name.at(1) < 123) // left hand id
		{
			// load accumulator from memory associated with the variabe
			runtime_environment[codePointer] = 173; // ad
			cpPP();
			runtime_environment[codePointer] = 0;
			addTemp(left, codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			// store accumulator in the unused memory address that is a part of the isntruction
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer + 1;
			cpPP();
			runtime_environment[codePointer] = 0; // value will be stored here
			leftBool = codePointer; // this is where to eventually get the left variable
			cpPP();
		}
		else if(left.name == "[true]" || left.name == "[false]")
		{
			int boolean = 0;
			if(left.name == "[true]") boolean = 1;
			// load accumulator with constant
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = boolean; // true or false | 1 or 0
			cpPP();
			// store accumulator in the unused memory address that is a part of the isntruction
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer + 1;
			cpPP();
			runtime_environment[codePointer] = 0; // value will be stored here
			leftBool = codePointer; // this is where to eventually get the left variable
			cpPP();
		}
		else if(left.name == "<+>")
		{
			generateCode(left, stringsMap); // recurse on <+>
			// after recursing, the accumulator should contain the correct number to compare
			// so all that needs to be done is to store the accumulator in memory
			// store accumulator in the unused memory address that is a part of the isntruction
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer + 1;
			cpPP();
			runtime_environment[codePointer] = 0; // value will be stored here
			leftBool = codePointer; // this is where to eventually get the left variable
			cpPP();
		}
		else // string literal 
		{
			string key = left.name.substr(2, left.name.length()-4);
			int addressOfString = stringsMap.at(key); // get the memory address of the string
			// load accumulator
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = addressOfString;
			cpPP();
			// store accumulator in the unused memory address that is a part of the isntruction
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer + 1;
			cpPP();
			runtime_environment[codePointer] = 0; // value will be stored here
			leftBool = codePointer; // this is where to eventually get the left variable
			cpPP();
		}
		
		// RIGHT HAND SIDE
		if(right.name == "<==>" || right.name == "<!=>")
		{
			generateCode(right, stringsMap); // recurse on <==> or <!=>
			// load x from memory
			runtime_environment[codePointer] = 174; // ae
			cpPP();
			runtime_environment[codePointer] = codePointer-2; // address of last boolean push to memory
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
		}
		else if(right.name.length() == 3 && right.name.at(1) > 47 && right.name.at(1) < 58) // right hand digit
		{
			int num = right.name.at(1) - 48;
			// load x with constant
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = num;
			cpPP();
		}
		else if(right.name.length() == 3 && right.name.at(1) > 96 && right.name.at(1) < 123) // right hand id
		{
			// load x from memory
			runtime_environment[codePointer] = 174; // ae
			cpPP();
			runtime_environment[codePointer] = 0;
			addTemp(right, codePointer); // temp var
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
		}
		else if(right.name == "[true]" || right.name == "[false]")
		{
			int boolean = 0;
			if(right.name == "[true]") boolean = 1;
			// load x with constant
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = boolean; // true or false | 1 or 0
			cpPP();
		}
		else if(right.name == "<+>")
		{
			generateCode(right, stringsMap); // recurse on <+>
			// after recursing, the accumulator should contain the correct number to compare
			// so all that needs to be done is to store the accumulator in memory
			// store accumulator in the unused memory address that is a part of the isntruction
			runtime_environment[codePointer] = 141; // 8d
			cpPP();
			runtime_environment[codePointer] = codePointer + 1;
			cpPP();
			runtime_environment[codePointer] = 0; // value will be stored here
			cpPP();
			// now we need to put the stored value into x register
			runtime_environment[codePointer] = 174; // ae
			cpPP();
			runtime_environment[codePointer] = codePointer - 2; // get stored value and put into x
			cpPP();
			runtime_environment[codePointer] = 0;
		}
		else // string literal 
		{
			string key = right.name.substr(2, children.at(1).name.length()-4);
			int addressOfString = stringsMap.at(key); // get the memory address of the string
			// store address in x register
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = addressOfString;
			cpPP();
		}
		
		// COMPARE SIDES
		// compare byte in memory to x register
		runtime_environment[codePointer] = 236; // ec
		cpPP();
		runtime_environment[codePointer] = leftBool; // memory address of left
		cpPP();
		runtime_environment[codePointer] = 0;
		cpPP();
		// branch 3 bytes if not equal
		runtime_environment[codePointer] = 208; //d0
		cpPP();
		runtime_environment[codePointer] = 3; // 3 bytes
		cpPP();
		if(name == "<!=>")
		{
			// true (z flag is 1) - load accumulator with contstant 1
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = 1;
			cpPP();
			// false (z flag is 0) - load accumulator with constant 0
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
		}
		else // name  == "<==>"
		{
			// true (z flag is 1) - load accumulator with contstant 0
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			// false (z flag is 0) - load accumulator with constant 1
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = 1;
			cpPP();
		}
		// store accumulator in the unused memory address that is a part of the isntruction
		runtime_environment[codePointer] = 141; // 8d
		cpPP();
		runtime_environment[codePointer] = codePointer + 1;
		cpPP();
		runtime_environment[codePointer] = 0; // value will be stored here
		cpPP();
	}
	else if(name == "<+>")
	{
		if(children.at(1).name.at(1) > 96 && children.at(1).name.at(1) < 123) // seocnd child is id and not <+>
		{
			value += children.at(0).name.at(1) - 48; // add left digit
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
			num += children.at(0).name.at(1) - 48; // add left digit
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
			value = 0; // reset value
		}
		else // second child is <+>
		{
			value += children.at(0).name.at(1) - 48; // add left digit
			generateCode(children.at(1), stringsMap); // recurse on <+>
		}
	}
	else if(name == "<IfStatement>")
	{
		AST_Node& conditional = children.at(0);
		AST_Node& then = children.at(1);
		if(conditional.name == "[false]")
		{
			cout << "[WARN]Line " << conditional.lineNum << ": " << "This for statement will never be executed." << endl;
			++numWarn;
		}
		else if(conditional.name == "[true]")
		{
			generateCode(then ,stringsMap); // we know it will evaluate so just compute the then part
		}
		else // <==> or <!=>
		{
			generateCode(conditional, stringsMap);
			// final value of a nested boolean expression will be stored in the last memory address
			// load the x register with a constant representing true
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1; // true
			cpPP();
			// compare x register with memory holding result of boolean expression
			runtime_environment[codePointer] = 236; // ec
			cpPP();
			runtime_environment[codePointer] = codePointer-4;
			cpPP();
			runtime_environment[codePointer] = 0;
			// branch n bytes if false
			runtime_environment[codePointer] = 208; // d0
			cpPP();
			runtime_environment[codePointer] = 1; // n starts at 1
			jumps.push_back(codePointer); // add this memory address to jump modifying queue
			cpPP();
			// evaluate the <block>
			generateCode(then, stringsMap);
			jumps.pop_back(); // remove the jump address from modifying queue
		}
	}
	else if(name == "<WhileStatement>")
	{
		AST_Node& conditional = children.at(0);
		AST_Node& then = children.at(1);
		if(conditional.name == "[false]")
		{
			cout << "[WARN]Line " << conditional.lineNum << ": " << "This loop will never be executed." << endl;
			++numWarn;
		}
		else if(conditional.name == "[true]")
		{
			cout << "[WARN]Line " << conditional.lineNum << ": " << "This language has no method of breaking from an iteration that loops on [true]." << endl;
			++numWarn;
			int savedAddress = codePointer; // save memory address to loop
			generateCode(then, stringsMap);
			// load accumulator with a 0
			runtime_environment[codePointer] = 169; // a9
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			// store accumulator in unused memory address of instruction
			runtime_environment[codePointer] = 141;
			cpPP();
			runtime_environment[codePointer] = codePointer+1;
			cpPP();
			runtime_environment[codePointer] = 0; // accumulator will be stored here
			cpPP();
			// load the x register with a constant representing true
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1; // true
			cpPP();
			// compare x with 0
			runtime_environment[codePointer] = 236; // ec
			cpPP();
			runtime_environment[codePointer] = codePointer-4;
			cpPP();
			runtime_environment[codePointer] = 0;
			cpPP();
			// branch all the way around
			runtime_environment[codePointer] = 208; // d0
			cpPP();
			runtime_environment[codePointer] = 256 - (codePointer-savedAddress);
			cpPP();
		}
		else // <==> or <!=>
		{
			generateCode(conditional, stringsMap);
			// final value of a nested boolean expression will be stored in the last memory address
			// load the x register with a constant representing true
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1; // true
			cpPP();
			// compare x register with memory holding result of boolean expression
			runtime_environment[codePointer] = 236; // ec
			cpPP();
			runtime_environment[codePointer] = codePointer-4;
			cpPP();
			runtime_environment[codePointer] = 0;
			int saved0Val = codePointer;
			cpPP();
			// branch n bytes if false
			runtime_environment[codePointer] = 208; // d0
			cpPP();
			runtime_environment[codePointer] = 1; // n starts at 1
			jumps.push_back(codePointer); // add this memory address to jump modifying queue
			cpPP();
			// evaluate the <block>
			generateCode(then, stringsMap);
			int loopBranch = 256 - runtime_environment[jumps.back()]; // how much to loop around
			jumps.pop_back(); // remove the jump address from modifying queue
			// load x register with a 1
			runtime_environment[codePointer] = 162; // a2
			cpPP();
			runtime_environment[codePointer] = 1;
			cpPP();
			// compare x register with memory holding 0 so loop around always happens
			runtime_environment[codePointer] = 236; // ec
			cpPP();
			runtime_environment[codePointer] = saved0Val; // this memory definitely contains a 0
			cpPP();
			runtime_environment[codePointer] = 0;
			// loop - branch all the way to beginning of loop
			runtime_environment[codePointer] = 208; // d0
			cpPP();
			runtime_environment[codePointer] = loopBranch; // loop around
			cpPP();
		}
	}
	/*
	else if(name.length() == 3 && name.at(1) > 47 && name.at(1) < 58) // [0-9]
	{
		int num = name.at(1) - 48;
		value += num; // add to the value
		return;
	}
	*/
	
	
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

// function to trace the creation of hex value
void Code_Generator::hexTrace()
{
	for(int i = 0; i < 256; ++i)
	{
		int op = runtime_environment[i];
		switch(op)
		{
			case 169: // A9
				cout << "Load accumulator with constant " << runtime_environment[++i] << endl;
				break;
			case 173: // AD
				cout << "Load accumulator with memory @ " << runtime_environment[++i] << endl;
				++i;
				break;
			case 141: // 8D
				cout << "Store accumulator in memory @ " << runtime_environment[++i] << endl;
				++i;
				break;
			case 109: // 6D
				cout << "Add with carry the value in memory @ " << runtime_environment[++i] << endl;
				++i;
				break;
			case 162: // A2
				cout << "Load the X register with constant " << runtime_environment[++i] << endl;
				break;
			case 174: // AE
				cout << "Load the X register with memory @ " << runtime_environment[++i] << endl;
				++i;
				break;
			case 160: // A0
				cout << "Load the Y register with constant " << runtime_environment[++i] << endl;
				break;
			case 172: // AC
				cout << "Load the Y register with memory @ " << runtime_environment[++i] << endl;
				++i;
				break;
			case 0: // 00
				// cout << "Break" << endl;
				break;
			case 236: // EC
				cout << "Set Z = 1 if X = memory @ " << runtime_environment[++i] << endl;
				++i;
				break;
			case 208: // D0
				cout << "Branch [" << runtime_environment[++i] << "] bytes if Z = 0" << endl; 
				break;
			case 255: // FF
				cout << "Print" << endl;
				break;
			default:
				// cout << "ERROR at location " << i << "!" << endl;
				break;
		}
	}
}

// function to conver the code into a string
void Code_Generator::create6502aCode()
{
	stringstream hexStream;
	for(int i = 0; i < 32; ++i)
	{
		int hex = i*8; // points to correct row
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
			hexStream << hexDigit1 << hexDigit2 << " ";
		}
	}
	hex = hexStream.str();
}