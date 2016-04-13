using namespace std;
using std::string;
using std::queue;
using std::vector;
using std::unordered_map;

// the Symbol structure (for the symbol table)
typedef struct Symbol
{
	string name; // name of the symbol
	string type; // the type of this symbol
	int lineNum; // the line number this symbol is found on
	int intVal; // integer value (set only if type=="int")
	bool boolVal; // boolean value (set only if type=="boolean")
	string stringVal; // string value (set only if type=="string")
	bool initialized; // default false; becomes true if symbol is initialized after declaration
	bool used; // default false; becomes true if symbol is used after initialization
	int scope; // scope of the symbol
} Symbol;

// the structure for each node of the symbol table
typedef struct Table_Node
{
	int scope; // the scope of this piece of the symbol table
	unordered_map<string, Symbol&> symbols; // symbol table hash map <id, symbol>
	Table_Node* parent; // the enclosing scope
} Table_Node;

// the AST node structure
typedef struct AST_Node
{
	string name; // the name of this node
	string type; // the type associated with this node	
	int scope; // the scope associated with this node
	int lineNum;
	vector<AST_Node>* children; // a queue containing the child nodes
} AST_Node;

class Semantic_Analyzer
{
	// public class access
	public:
		Semantic_Analyzer(Node&, bool); // constructor
		AST_Node AST; // the abstract syntax tree
		int numErrors; // number of errors found
		int numWarn; // number of warnings found
	// private class access
	private:
		bool verbose;
		void constructAST(Node&, queue<Node>&, int);
		AST_Node resolveTypes(Node&, queue<Node>&);
		void constructSymbolTable(AST_Node&, Table_Node*, queue<Symbol*>&, int);
		void printAST(AST_Node, int);
		Node nmake(string, string, int, int);
		void typeCheck(AST_Node&);
};

// constructor
Semantic_Analyzer::Semantic_Analyzer(Node& CST, bool v)
{
	verbose = v;
	numErrors = 0; // start with no errors, of course
	numWarn = 0; // start with no warnings, of course
	queue<Node> AST_queue;
	constructAST(CST, AST_queue, 0);
	if(!AST_queue.empty())
	{
		// resolves types, except for individual ids, while at the same time constructing a new AST using vectors instead of queues
		AST = resolveTypes(AST_queue.front(), *AST_queue.front().children);
	}
	
	// we'll need to store each symbol table node in this queue
	// so we can iterate through it and print out all the symbols
	// since iterating through a tree where pointers are to parent
	// nodes instead of child nodes is a bit trickier
	queue<Symbol*> symTblPrntQ; // symbol table print queue
	constructSymbolTable(AST, nullptr, symTblPrntQ, 0); // construct the symbol table
	queue<Symbol*> savedQ = symTblPrntQ; // save the queue for second traversal
	
	// type check
	typeCheck(AST);
	
	// traverse the table to warn about unused variables
	while(!symTblPrntQ.empty())
	{
		Symbol sym = *symTblPrntQ.front();
		// warn the user if a variable is never used
		if(!sym.used)
		{
			string init = (sym.initialized) ? "initialized" : "uninitialized"; // is the variable at least initialized? modify the warning
			cout << "[WARN]Line " << sym.lineNum << ": " << "The " << init << 
			" variable " << sym.name << " is never used." << endl;
			++numWarn;
		}
		symTblPrntQ.pop();
	}
	
	if(verbose) // if verbose mode is on
	{
		cout <<
			"______________________________________________________________________" << endl <<
			setw(25) << left << "" << "ABSTRACT SYNTAX TREE" << setw(25) << right << "" << endl <<
			"______________________________________________________________________" << endl;
		printAST(AST, 0); // print the AST
		cout << "______________________________________________________________________" << endl;
		// print symbol table
		cout <<
			"______________________________________________________________________" << endl <<
			setw(29) << left << "" << "SYMBOL TABLE" << setw(29) << right << "" << endl <<
			"______________________________________________________________________" << endl;
		while(!savedQ.empty())
		{
			Symbol sym = *savedQ.front();
			string type = sym.type;
			// rename type so there are no square brackets
			if(type == "[int]") type = "int";
			else if(type == "[string]") type = "string";
			else type = "boolean";
			// print out symbol table
			cout << left <<
				"[NAME: " << setw(5) << sym.name.substr(1,1) << "]" << 
				"[TYPE: " << setw(10) << type << "]" << 
				"[SCOPE: " << setw(12) << sym.scope << "]" <<
				"[LINE: "<< setw(10) << sym.lineNum << "]" << endl;
			savedQ.pop();
		}
		cout << "______________________________________________________________________" << endl;
	}
}

// type checks the AST
// &node	: the current node being analyzed
void Semantic_Analyzer::typeCheck(AST_Node& node)
{
	string& name = node.name;
	vector<AST_Node>& children = *node.children;
	int lineNum;
	if(children.size() > 1 && children.at(1).type == "id") 
		children.at(1).type = "void"; // change id type to void since this is an undeclared id in my underlying logic
	if(name == "<AssignmentStatement>") // must check assignment's type matches variable's type
	{
		if(children.at(0).type != children.at(1).type)
		{
			lineNum = children.at(0).lineNum;
			cout << "[ERROR]Line " << lineNum << ": (Type Mismatch) " << "The variable " << children.at(0).name <<
				" can only be assigned a type of " << children.at(0).type << ", not " << children.at(1).type << "." <<endl;
			++numErrors;
		}
	}
	else if(name == "<+>") // must check right child node has a type of int
	{
		if(children.at(1).type != "int")
		{
			lineNum = children.at(1).lineNum;
			cout << "[ERROR]Line " << lineNum << ": (Type Mismatch) " << "Only integers may be added together; a " <<
				children.at(1).type << " was found in your addition equation." << endl;
			++numErrors;
		}
	}
	else if(name == "<==>" || name == "<!=>") // must check both child nodes have the same type
	{
		if(children.at(0).type != children.at(1).type)
		{
			lineNum = children.at(0).lineNum;
			string grammar1 = (children.at(0).type == "int") ? "An " : "A ";
			string grammar2 = (children.at(1).type == "int") ? "an " : "a ";
			cout << "[ERROR]Line " << lineNum << ": (Type Mismatch) " << grammar1 << children.at(0).type << " " <<
				" cannot be compared to " << grammar2 << children.at(1).type << " " <<
				"." << endl;
			++numErrors;
		}
	}
	for(vector<AST_Node>::iterator it=children.begin(); it != children.end(); ++it) // for each child node
		typeCheck(*it); // recurse
}

// assigns types to each node of the AST
// NOTE: types of variables (ids) are not assigned here, but are instead in constructSymbolTable
// &node	 : the node being analyzed
// $children : the children of &node
// returns	 : a root AST_Node of the entire AST
AST_Node Semantic_Analyzer::resolveTypes(Node& node, queue<Node>& children)
{
	string name = node.name;
	AST_Node newNode;
	newNode.name = name;
	newNode.type = node.type;
	newNode.scope = node.scope;
	newNode.lineNum = node.lineNum;
	newNode.children = new vector<AST_Node>;
		
	if(name == "[int]" || name == "[string]" || name == "[boolean]" || name == "[true]" || name == "[false]") // obvious types
	{
		if(name == "[int]") newNode.type = "int";
		else if(name == "[string]") newNode.type = "string";
		else if(name == "[boolean]" || name == "[true]" || name == "[false]") newNode.type = "boolean";
	}
	else if(!children.empty()) // if this node has children we'll have to recurse until we find the type for it
	{
		vector<AST_Node>*& newChildren = newNode.children; // the new AST will contain vector children instead of queues for better traversal
		string newType = "void"; // default type
		while(!children.empty()) // recurse through the children
		{
			AST_Node n = resolveTypes(children.front(), *children.front().children);
			(*newChildren).push_back(n);
			if(n.type == "int" || n.type == "string" || n.type == "boolean")
				newType = n.type; // the new AST_Node will have the same type as the children
			children.pop();
		}
		newNode.type = newType;
		newNode.children = newChildren;
	}
	return newNode;
}

// function to construct the symbol table and also catch scope & type errors along the way
// n				: the current node in the AST being analyzed
// *tn				: the symbol table node we are currently adding symbols to
// &symTblPrintQ	: the print queue for all symbols
// scope			: the current scope
void Semantic_Analyzer::constructSymbolTable(AST_Node& n, Table_Node* tn, queue<Symbol*>& symTblPrntQ, int scope)
{
	Table_Node* toPass = tn; // table node to pass recursively
	Table_Node& curTN = *tn; // let's us work with the actual table node
	vector<AST_Node>& children = *n.children; // get the node's children
	
	if(n.name == "<Block>") // new scope
	{
		unordered_map<string, Symbol&> symbols;
		Table_Node* newTN = new Table_Node{++scope, symbols, tn};
		toPass = newTN;
	}
	else if(n.name == "<VarDecl>") // variable declaration - add symbol
	{
		int lineNum = children.front().lineNum; // line number of the symbol
		string type = children.front().name; // int/string/boolean
		string key = children.at(1).name; // the variable
		Symbol* sPointer = new Symbol{key, type, lineNum, 0, false, "", false, false, scope}; // create a new symbol with the type
		Symbol& symbol = *sPointer;
		if(curTN.symbols.emplace(key, symbol).second == false) // add the symbol to the symbol table node if it doesn't yet exist
		{
			cout << "[ERROR]Line " << lineNum << ": " << "The variable " << key <<
			" was already declared in this scope on line " << curTN.symbols.at(key).lineNum << "." << endl;
			++numErrors; // increment the number of errors found
		}
		else
		{
			symTblPrntQ.push(sPointer); // add symbol to print queue
			children.at(1).type = type.substr(1, type.length()-2); // assign the variable AST_Node its type here
		}
		return; // don't continue analyzing the child nodes
	}
	else if(n.name == "<AssignmentStatement>") // need to make sure this variable has been declred
	{
		AST_Node& var = children.front(); // left hand side of expression (the variable)
		Table_Node* scopeChecker = tn; // start checking this scope, but also move up to enclosing scopes
		while(scopeChecker != nullptr) // while there is an enclosing scope to check
		{
			if((*scopeChecker).symbols.count(var.name) == 1) // if the variable was declared in this scope
			{	
				Symbol& sym = (*scopeChecker).symbols.at(var.name);
				sym.initialized = true;
				var.type = sym.type.substr(1, sym.type.length()-2); // assign the variable AST_Node its type here
				constructSymbolTable(children.at(1), toPass, symTblPrntQ, scope); // recurse on right side of assignment statement
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
		Table_Node*& scopeChecker = tn; // start checking this scope, but also move up to enclosing scopes
		while(scopeChecker != nullptr) // while there is an enclosing scope to check
		{
			if((*scopeChecker).symbols.count(n.name) == 1) // if the variable was declared in this scope
			{	
				Symbol& sym = (*scopeChecker).symbols.at(n.name);
				n.type = sym.type.substr(1, sym.type.length()-2); // assign the AST_Node its type here
				if(sym.initialized == true) 
				{
					sym.used = true; // we now know the symbol has been used at least once
				}
				else
				{
					cout << "[WARN]Line " << n.lineNum << ": " << "The variable " << n.name <<
						" has not been initialized within this scope." << endl;
					++numWarn;
				}
				return; // don't go on to report variable not declared, as it was
			}			
			scopeChecker = (*scopeChecker).parent; // go to next enclosing scope
		}
		// if the symbol was not declared nor initialized in scope
		cout << "[ERROR]Line " << n.lineNum << ": " << "The variable " << n.name <<
			" has neither been initialized nor declared within this scope." << endl;
		++numErrors;
		return; // don't continue analyzing the child nodes
	}
	
	for(vector<AST_Node>::iterator it=children.begin(); it != children.end(); ++it) // for each child node
	{
		constructSymbolTable(*it, toPass, symTblPrntQ, scope); // recurse
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
void Semantic_Analyzer::printAST(AST_Node n, int level)
{
	for(int i=0; i < level; ++i) // for the node's depth
		cout << "-"; // print out a corresponding number of dashes
	cout << n.name <<
	// "(Line No. " << n.lineNum << ")" << // print node's line number 
	// "(Type " << n.type << ")" << // print node's name
	endl;
	vector<AST_Node> children = *n.children; // get the node's children
	for(vector<AST_Node>::iterator it = children.begin(); it != children.end(); ++it) // for each child node
	{
		printAST(*it, level+1); // recurse
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
		if(name == "<CharList>") node.type = "string";
		if(name == "<boolop>") node.name = "<" + node.name.substr(1, node.name.length()-2) + ">"; // modify the boolop a little for the AST
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
		if(type == "digit") type = "int";
		AST.push(nmake(name, type, scope, lineNum));
		return;
	}
}