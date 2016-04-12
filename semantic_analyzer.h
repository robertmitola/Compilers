using namespace std;
using std::string;
using std::queue;
using std::unordered_map;

// the Symbol structure (for the symbol table)
typedef struct Symbol
{
	string type; // the type of this symbol
	int lineNum; // the line number this symbol is found on
	int intVal; // integer value (set only if type=="int")
	bool boolVal; // boolean value (set only if type=="boolean")
	string stringVal; // string value (set only if type=="string")
	bool initialized; // default false; becomes true if symbol is initialized after declaration
	bool used; // default false; becomes true if symbol is used after initialization
} Symbol;

// the structure for each node of the symbol table
typedef struct Table_Node
{
	int scope; // the scope of this piece of the symbol table
	unordered_map<string, Symbol> symbols; // symbol table hash map <id, symbol>
	Table_Node* parent; // the enclosing scope
} Table_Node;

class Semantic_Analyzer
{
	// public class access
	public:
		Semantic_Analyzer(Node&, bool); // constructor
		Node AST; // the abstract syntax tree
		int numErrors; // was an error found?
	// private class access
	private:
		bool verbose;
		void constructAST(Node&, queue<Node>&, int);
		void constructSymbolTable(Node, Table_Node*, queue<Table_Node>&, int);
		void printAST(Node, int);
		Node nmake(string, string, int, int);
};

// constructor
Semantic_Analyzer::Semantic_Analyzer(Node& CST, bool v)
{
	verbose = v;
	numErrors = 0; // start with no errors, of course
	queue<Node> AST_queue;
	constructAST(CST, AST_queue, 0);
	if(!AST_queue.empty())
	{
		AST = AST_queue.front();
	}
	if(verbose) // if verbose mode is on
	{
		cout << endl << "The Abstract Syntax Tree:" << endl;
		printAST(AST, 0); // output the CST
	}
	
	// we'll need to store each symbol table node in this queue
	// so we can iterate through it and print out all the symbols
	// since iterating through a tree where pointers are to parent
	// nodes instead of child nodes is a bit trickier
	queue<Table_Node> symTblPrntQ; // symbol table print queue

	constructSymbolTable(AST, nullptr, symTblPrntQ, 0);
	
}


void Semantic_Analyzer::constructSymbolTable(Node n, Table_Node* tn, queue<Table_Node>& symTblPrntQ, int scope)
{
	Table_Node* toPass = tn; // table node to pass recursively
	Table_Node& curTN = *tn; // let's us work with the actual table node
	queue<Node> children = *n.children; // get the node's children
	
	if(n.name == "<Block>") // new scope
	{
		unordered_map<string, Symbol> symbols;
		Table_Node* newTN = new Table_Node{++scope, symbols, tn};
		symTblPrntQ.push(*newTN); // add this table node to the queue of table nodes
		toPass = newTN;
	}
	else if(n.name == "<VarDecl>") // variable declaration - add symbol
	{
		int lineNum = children.front().lineNum; // line number of the symbol
		string type = children.front().name; // int/string/boolean
		children.pop(); // remove type
		string key = children.front().name; // the variable
		Symbol symbol = {type, lineNum, 0, false, "", false, false}; // create a new symbol with the type
		if(curTN.symbols.emplace(key, symbol).second == false) // add the symbol to the symbol table node if it doesn't yet exist
		{
			cout << "[ERROR]Line " << lineNum << ": " << "The variable " << key <<
			" was already declared in this scope on line " << curTN.symbols.at(key).lineNum << "." <<endl;
			++numErrors; // increment the number of errors found
		}
		return; // don't continue analyzing the child nodes
	}
	else if(n.name == "<AssignmentStatement>") // need to make sure this variable has been declred
	{
		Node var = children.front(); // left hand side of expression (the variable)
		children.pop();
		Table_Node* scopeChecker = tn; // start checking this scope, but also move up to enclosing scopes
		while(scopeChecker != nullptr) // while there is an enclosing scope to check
		{
			if((*scopeChecker).symbols.count(var.name) == 1) // if the variable was declared in this scope
			{	
				Symbol& sym = (*scopeChecker).symbols.at(var.name);
				sym.initialized = true;
				constructSymbolTable(children.front(), toPass, symTblPrntQ, scope); // recurse on right side of assignment statement
				return; // symbol was indeed declared - no problems
			}			
			scopeChecker = (*scopeChecker).parent; // go to next enclosing scope
		}
		// if the symbol was not declared in scope
		cout << "[ERROR]Line " << var.lineNum << ": " << "The variable " << var.name <<
			" has not been declared within this scope." << endl;
		++numErrors;
		return; // don't continue analyzing the child nodes
	}
	else if(n.type == "id") // need to make sure the variable was initialized
	{
		Table_Node* scopeChecker = tn; // start checking this scope, but also move up to enclosing scopes
		while(scopeChecker != nullptr) // while there is an enclosing scope to check
		{
			if((*scopeChecker).symbols.count(n.name) == 1) // if the variable was declared in this scope
			{	
				Symbol& sym = (*scopeChecker).symbols.at(n.name);
				if(sym.initialized == true) 
				{
					return; // symbol was indeed initialized - no problems
				}
				else
				{
					cout << "[ERROR]Line " << n.lineNum << ": " << "The variable " << n.name <<
						" has not been initialized within this scope." << endl;
					++numErrors;
					return; // don't go on to report variable not declared, as it was
				}
			}			
			scopeChecker = (*scopeChecker).parent; // go to next enclosing scope
		}
		// if the symbol was not declared nor initialized in scope
		cout << "[ERROR]Line " << n.lineNum << ": " << "The variable " << n.name <<
			" has neither been initialized nor declared within this scope." << endl;
		++numErrors;
		return; // don't continue analyzing the child nodes
	}
	
	while(!children.empty()) // for each child node
	{
		constructSymbolTable(children.front(), toPass, symTblPrntQ, scope); // recurse
		children.pop();
	}
}

// function to make a node to push onto the current AST queue
// name		: name of the node
// type		: data type associated with the node
// scope	: scope of the node
// lineNum	: the line number
// returns	: a node
Node Semantic_Analyzer::nmake(string name, string type, int scope, int lineNum)
{
	queue<Node>* children = new queue<Node>;
	Node n = {name, type, scope, lineNum, children}; // initialize with type void and scope 0 to be changed during AST creation
	return n;
}

// prints out the abstract syntax tree
// n		: the current node
// level	: the depth of this node
void Semantic_Analyzer::printAST(Node n, int level)
{
	for(int i=0; i < level; ++i) // for the node's depth
		cout << "-"; // print out a corresponding number of dashes
	cout << n.name <<
	"(Line No. " << n.lineNum << ")" << 
	"(Scope " << n.scope << ")" << endl; // print node's name
	queue<Node> children = *n.children; // get the node's children
	while(!children.empty()) // for each child node
	{
		printAST(children.front(), level+1); // recurse
		children.pop();
	}
}

// function to construct an AST from a CST
// node		: the current node being analyzed
// nodes	: the queue of child nodes we are adding to
// scope	: scope of this node
void Semantic_Analyzer::constructAST(Node& node, queue<Node>& AST, int scope)
{
	string& name = node.name;
	string& type = node.type;
	int& lineNum = node.lineNum;
	queue<Node>& children = *node.children;
	
	// analyze the given node of the CST and determine:
	//	1) Should this node go on the AST? For some nodes, such as <BooleanExpr>, this is conditional.
	//	2) What child nodes of this node are important for the AST?
	if(name == "<Program>")
	{
		Node& n = children.front();	// <Block>			
		constructAST(n, AST, scope);
	}
	else if(name == "<Block>")
	{
		// cout << "Analyzing block" << endl;
		Node block = nmake(name, type, scope, lineNum);
		AST.push(block);
		children.pop(); // remove [{]
		Node& n = children.front(); // <StatementList>
		constructAST(n, *block.children, scope+1);
	}
	else if(name == "<StatementList>")
	{
		// cout << "Analyzing stmt list" << endl;
		while(!children.empty())
		{
			Node& n = children.front();
			constructAST(n, AST, scope);
			children.pop();
		}
		return;
	}
	else if(name == "<Statement>" || name == "<Expr>")
	{
		// cout << "Analyzing stmt" << endl;
		Node& n = children.front(); // can only have one child node
		constructAST(n, AST, scope);
	}
	else if(name == "<PrintStatement>")
	{
		// cout << "Analyzing print stmt" << endl;
		Node print = nmake(name, type, scope, lineNum);
		AST.push(print);
		children.pop(); // remove [print]
		children.pop(); // remove [(]
		Node& n = children.front(); // <Expr> to print
		constructAST(n, *print.children, scope);
	}
	else if(name == "<AssignmentStatement>")
	{
		// cout << "Analyzing assign stmt" << endl;
		Node assign = nmake(name, type, scope, lineNum);
		AST.push(assign);
		Node& n = children.front(); // [id]
		constructAST(n, *assign.children, scope);
		children.pop(); // remove [id]
		children.pop(); // remove [=]
		n = children.front(); // <Expr>
		constructAST(n, *assign.children, scope);
	}
	else if(name == "<VarDecl>")
	{
		// cout << "Analyzing var decl" << endl;
		Node varDecl = nmake(name, type, scope, lineNum);
		AST.push(varDecl);
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<type>")
			{
				constructAST(n, *varDecl.children, scope);
				while(!children.empty())
				{
					n = children.front();
					if(n.type == "id")
					{
						constructAST(n, *varDecl.children, scope);
						return;
					}
					children.pop();
				}
			}
			children.pop();
		}
	}
	else if(name == "<WhileStatement>" || name == "<IfStatement>")
	{
		// cout << "Analyzing while/if stmt" << endl;
		Node node = nmake(name, type, scope, lineNum);
		AST.push(node);
		children.pop(); // remove [wile] / [if]
		Node& n = children.front(); // <BooleanExpr>
		constructAST(n, *node.children, scope);
		children.pop(); // remove <BooleanExpr>
		n = children.front(); // <Block>
		constructAST(n, *node.children, scope);
	}
	else if(name == "<type>" || name == "<CharList>" || name == "<boolop>" || name == "<boolval>")
	{
		// cout << "Analyzing type/char list/boolop/boolval" << endl;
		queue<Node>& children = *node.children;
		Node& node = children.front();
		AST.push(nmake(node.name, node.type, scope, node.lineNum));
		return;
	}
	else if(name == "<IntExpr>")
	{
		// cout << "Analyzing int expr" << endl;
		Node& n = children.front();
		if(children.size() == 1) // if the int expr is just a digit
			constructAST(n, AST, scope);
		else
		{
			Node integer = nmake("<+>", "int", scope, lineNum);
			AST.push(integer);
			constructAST(n, *integer.children, scope);
			children.pop(); // remove [digit]
			children.pop(); // remove [+]
			n = children.front(); // <expr>
			constructAST(n, *integer.children, scope);
		}
		return;
	}
	else if(name == "<StringExpr>")
	{
		// cout << "Analyzing string expr" << endl;
		children.pop(); // remove ["]
		Node& n = children.front(); // <CharList>
		constructAST(n, AST, scope);
	}
	else if(name == "<BooleanExpr>")
	{
		// cout << "Analyzing bool expr" << endl;
		Node& n = children.front(); // <boolop>
		if(children.size() == 1) // if the only child node is <boolval>
		{
			constructAST(n, AST, scope);
		}
		else // we found (Expr boolop Expr)
		{
			Node boolean = nmake(name, "boolean", scope, lineNum);
			children.pop(); // remove [(]
			Node n1 = children.front(); // first <Expr>
			children.pop(); // remove <Expr>
			n = children.front(); // <boolop>
			constructAST(n, *boolean.children, scope);
			children.pop(); // remove <boolop>
			Node n2 = children.front(); // second <Expr>
			// logic for changing the name of the boolean node to match the given boolean operator
			queue<Node>& nodes = *boolean.children;
			boolean.name = nodes.front().name; // change name to boolean operator
			nodes.pop(); // remove boolean operator
			constructAST(n1, *boolean.children, scope); // add first node to compare
			constructAST(n2, *boolean.children, scope); // add second node to compare
			AST.push(boolean);
		}
	}
	else if(name == "[true]" || name == "[false]" || name == "[==]" || name == "[!=]")
	{
		// cout << "Analyzing true/false/==/!=" << endl;
		AST.push(nmake(name, "boolean", scope, lineNum));
		return;
	}
	else if(name == "[+]")
	{
		// cout << "Analyzing +" << endl;
		AST.push(nmake(name, "int", scope, lineNum));
		return;
	}
	// these nodes should never even pass through this function, but this is here to catch them in case they do
	else if(name == "[if]" ||
			name == "[int]" ||
			name == "[boolean]" ||
			name == "[string]" ||
			name == "[print]" ||
			name == "[while]" ||
			name == "[\"]" ||
			name == "[(]" ||
			name == "[)]" ||
			name == "[{]" ||
			name == "[}]" ||
			name == "[=]" ||
			name == "[$]" ||
			name == "[epsilon]")
	{
		// cout << "Analyzing discardable node" << endl;
		return; // do nothing
	}
	else // id, digit
	{
		// cout << "Analyzing id/digit" << endl;
		AST.push(nmake(name, type, scope, lineNum));
		return;
	}
}