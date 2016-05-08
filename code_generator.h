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
	// private class access
	private:
		bool verbose; // verbose output
};

// constructor
Code_Generator::Code_Generator(AST_Node& AST, bool v)
{
	verbose = v; // set verbose to on or off
	numErrors = 0; // start with no errors
}