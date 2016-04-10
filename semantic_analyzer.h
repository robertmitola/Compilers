using namespace std;
using std::string;
using std::queue;

class Semantic_Analyzer
{
	// public class access
	public:
		Semantic_Analyzer(Node&, bool); // constructor
		Node AST; // the abstract syntax tree
	// private class access
	private:
		bool verbose;
		void constructAST(Node&, queue<Node>&, int);
		void printAST(Node, int);
		Node nmake(string, string, int);
};

Semantic_Analyzer::Semantic_Analyzer(Node& CST, bool v)
{
	verbose = v;
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
}

// function to make a node to push onto the current AST queue
// name		: name of the node
// type		: data type associated with the node
// scope	: scope of the node
// AST		: the current AST queue
// returns	: a node
Node Semantic_Analyzer::nmake(string name, string type, int scope)
{
	queue<Node>* children = new queue<Node>;
	Node n = {name, type, scope, children}; // initialize with type void and scope 0 to be changed during AST creation
	return n;
}

// prints out the abstract syntax tree
// n		: the current node
// level	: the depth of this node
void Semantic_Analyzer::printAST(Node n, int level)
{
	for(int i=0; i < level; ++i) // for the node's depth
		cout << "-"; // print out a corresponding number of dashes
	cout << n.name << "(" << n.scope << ")" << endl; // print node's name
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
	queue<Node>& children = *node.children;
	
	if(name == "<Program>")
	{
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<Block>")
			{				
				constructAST(n, AST, scope);
				return;
			}
			children.pop();
		}
	}
	else if(name == "<Block>")
	{
		cout << "Analyzing block" << endl;
		Node block = nmake(name, type, scope);
		AST.push(block);
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<StatementList>")
			{
				constructAST(n, *block.children, scope+1);
				return;
			}
			children.pop();
		}
	}
	else if(name == "<StatementList>")
	{
		cout << "Analyzing stmt list" << endl;
		while(!children.empty())
		{
			Node& n = children.front();
			constructAST(n, AST, scope);
			children.pop();
		}
		return;
	}
	else if(name == "<Statement>")
	{
		cout << "Analyzing stmt" << endl;
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<PrintStatement>" ||
				n.name == "<AssignmentStatement>" ||
				n.name == "<VarDecl>" ||
				n.name == "<WhileStatement>" ||
				n.name == "<IfStatement>" ||
				n.name == "<Block>")
			{
				constructAST(n, AST, scope);
			}
			children.pop();
		}
		
		return;
	}
	else if(name == "<PrintStatement>")
	{
		cout << "Analyzing print stmt" << endl;
		Node print = nmake(name, type, scope);
		AST.push(print);
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<Expression>")
			{
				constructAST(n, *print.children, scope);
				return;
			}
			children.pop();
		}
	}
	else if(name == "<AssignmentStatement>")
	{
		cout << "Analyzing assign stmt" << endl;
		Node assign = nmake(name, type, scope);
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
		cout << "Analyzing var decl" << endl;
		Node varDecl = nmake(name, type, scope);
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
		cout << "Analyzing while/if stmt" << endl;
		Node node = nmake(name, type, scope);
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
		cout << "Analyzing type/char list/boolop/boolval" << endl;
		queue<Node>& children = *node.children;
		Node& node = children.front();
		AST.push(nmake(node.name, node.type, scope));
		return;
	}
	else if(name == "<Expr>")
	{
		cout << "Analyzing expr" << endl;
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<IntExpr>" || n.name == "<StringExpr>" || n.name == "<BooleanExpr>" || n.type == "id")
			{
				constructAST(n, AST, scope);
				return;
			}
			children.pop();
		}
	}
	else if(name == "<IntExpr>")
	{
		cout << "Analyzing int expr" << endl;
		Node& n = children.front();
		if(children.size() == 1) // if the int expr is just a digit
			constructAST(n, AST, scope);
		else
		{
			Node integer = nmake("<+>", "int", scope);
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
		cout << "Analyzing string expr" << endl;
		while(!children.empty())
		{
			Node& n = children.front();
			if(n.name == "<CharList>")
			{
				constructAST(n, AST, scope);
				return;
			}
			children.pop();
		}
		return;
	}
	else if(name == "<BooleanExpr>")
	{
		cout << "Analyzing bool expr" << endl;
		Node& n = children.front();
		if(children.size() == 1) // if the only child node is <boolval>
		{
			constructAST(n, AST, scope);
		}
		else // we found (Expr boolop Expr)
		{
			Node boolean = nmake(name, "boolean", scope);
			AST.push(boolean);
			children.pop(); // remove [(]
			n = children.front(); // first <Expr>
			constructAST(n, *boolean.children, scope);
			children.pop(); // remove <Expr>
			n = children.front(); // <boolop>
			constructAST(n, *boolean.children, scope);
			children.pop(); // remove <boolop>
			n = children.front(); // second <Expr>
			constructAST(n, *boolean.children, scope);
		}
	}
	else if(name == "[if]")
	{
		cout << "Analyzing if" << endl;
		return;
	}
	else if(name == "[int]")
	{
		cout << "Analyzing int" << endl;
		return;
	}
	else if(name == "[boolean]")
	{
		cout << "Analyzing boolean" << endl;
		return;
	}
	else if(name == "[string]")
	{
		cout << "Analyzing string" << endl;
		return;
	}
	else if(name == "[print]")
	{
		cout << "Analyzing print" << endl;
		return;
	}
	else if(name == "[while]")
	{
		cout << "Analyzing while" << endl;
		return;
	}
	else if(name == "[true]" || name == "[false]" || name == "[==]" || name == "[!=]")
	{
		cout << "Analyzing true/false/==/!=" << endl;
		AST.push(nmake(name, "boolean", scope));
		return;
	}
	else if(name == "[\"]")
	{
		cout << "Analyzing \"" << endl;
		return;
	}
	else if(name == "[(]")
	{
		cout << "Analyzing (" << endl;
		return;
	}
	else if(name == "[)]")
	{
		cout << "Analyzing )" << endl;
		return;
	}
	else if(name == "[{]")
	{
		cout << "Analyzing {" << endl;
		return;
	}
	else if(name == "[}]")
	{
		cout << "Analyzing }" << endl;
		return;
	}
	else if(name == "[=]")
	{
		cout << "Analyzing =" << endl;
		return;
	}
	else if(name == "[+]")
	{
		cout << "Analyzing +" << endl;
		AST.push(nmake(name, "int", scope));
		return;
	}
	else if(name == "[$]")
	{
		cout << "Analyzing $" << endl;
		return;
	}
	else if(name == "[epsilon]")
	{
		cout << "Analyzing epsilon" << endl;
		return;
	}
	else // id, digit
	{
		cout << "Analyzing id/digit" << endl;
		AST.push(nmake(name, type, scope));
		return;
	}
}