using namespace std;
using std::string;
using std::queue;
using std::vector;
using std::unordered_map;

class Code_Generator
{
	// public class access
	public:
		Code_Generator(AST_Node&, bool); // constructor
		int numErrors; // number of errors
		int numWarn; // number of warnings
		int runtime_environment [256]; // the runtime environment array
	// private class access
	private:
		bool verbose; // verbose output
		int codePointer; // points to where code goes in the runtime environment
		int stopPointer; // points to where code should stop in the runtime environment
		void printRuntimeEnvironment(); // prints the runtime environment out
};

// constructor
Code_Generator::Code_Generator(AST_Node& AST, bool v)
{
	// variable initialization
	verbose = v; // set verbose to on or off
	numErrors = 0; // start with no errors
	codePointer = 0; // start code at first byte
	stopPointer = 255; // stop at byte 255 to start - this will change as string literals are added
	// fill the runtime environment with 00s
	for(int i = 0; i < 256; ++i)
		runtime_environment[i] = 0;
	
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
