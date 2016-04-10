using namespace std;
using std::string;
using std::queue;

// the CST node structure
typedef struct Node
{
	string name; // the name of this node
	string type; // the type associated with this node	
	int scope; // the scope associated with this node
	queue<Node>* children; // a queue containing the child nodes
} Node;

// the Parser class object definition
class Parser
{
	// public class access
	public:
		Parser(queue<Token>&, bool); // constructor
		int numErrors; // number of parser errors
		Node CST; // the concrete syntax tree
	// private class access
	private:
		string error; // the error message to return
		string stmtError; // error message for incorrect statement, since there are many different types of statements
		bool verbose; // verbose output?
		int errorLine; // line number of the error
		int stmtErrorLine; // line number of statement error
		bool charList; // true if we are inside a charList
		Token hpop(queue<Token>&);
		Node nmake(string);
		void printCST(Node, int);
		bool parseProgram(queue<Token>&);
		bool parseBlock(queue<Token> &, queue<Node>&);
		bool parseStatementList(queue<Token>&, queue<Node>&);
		bool parseStatement(queue<Token>&, queue<Node>&);
		bool parsePrintStatement(queue<Token>&, queue<Node>&);
		bool parseAssignmentStatement(queue<Token>&, queue<Node>&);
		bool parseVarDecl(queue<Token>&, queue<Node>&);
		bool parseWhileStatement(queue<Token>&, queue<Node>&);
		bool parseIfStatement(queue<Token>&, queue<Node>&);
		bool parseType(queue<Token>&, queue<Node>&);
		bool parseExpression(queue<Token>&, queue<Node>&);
		bool parseIntExpression(queue<Token>&, queue<Node>&);
		bool parseStringExpression(queue<Token>&, queue<Node>&);
		bool parseBooleanExpression(queue<Token>&, queue<Node>&);
		bool parseBoolOp(queue<Token>&, queue<Node>&);
		bool parseCharList(queue<Token>&, queue<Node>&);
		bool parseBoolval(queue<Token>&, queue<Node>&);
		bool matchT_EOF(Token, queue<Node>&);
		bool matchT_PLUS(Token, queue<Node>&);
		bool matchT_OPEN_BRACE(Token, queue<Node>&);
		bool matchT_CLOSE_BRACE(Token, queue<Node>&);
		bool matchT_OPEN_PAREN(Token, queue<Node>&);
		bool matchT_CLOSE_PAREN(Token, queue<Node>&);
		bool matchT_QUOTE(Token, queue<Node>&);
		bool matchT_EQUALS(Token, queue<Node>&);
		bool matchT_NOT_EQUALS(Token, queue<Node>&);
		bool matchT_FALSE(Token, queue<Node>&);
		bool matchT_TRUE(Token, queue<Node>&);
		bool matchT_WHILE(Token, queue<Node>&);
		bool matchT_PRINT(Token, queue<Node>&);
		bool matchT_ID(Token, queue<Node>&);
		bool matchT_STRING(Token, queue<Node>&);
		bool matchT_BOOLEAN(Token, queue<Node>&);
		bool matchT_IF(Token, queue<Node>&);
		bool matchT_SPACE(Token, queue<Node>&);
		bool matchT_ASSIGN(Token, queue<Node>&);
		bool matchT_DIGIT(Token, queue<Node>&);
		bool matchT_INT(Token, queue<Node>&);
};

// the Parser constructor
// que		: reference to the token queue the Parser should use
// v		: true if verbose output should happen
Parser::Parser(queue<Token>& que, bool v)
{
	numErrors = 0; // no errors at the start
	error = ""; // set error to nothing
	stmtError= ""; // set statement error to nothing
	verbose = v; // should verbose output happen?
	CST = nmake("<Program>"); // make root node of the CST
	if (!parseProgram(que))
	{
		if(stmtError != "") // if there was a semi-successful statement
		{
			error = stmtError; // error message should match the error for the most likely statement
			errorLine = stmtErrorLine; // error line should match the error line for the most likely statement
		}
		cout << "[ERROR]Line " << errorLine << ": " << error << endl; // report the error
		++numErrors; // increment the number of parser errors
	}
	if(verbose) // if verbose mode is on
	{
		cout << endl << "The Concrete Syntax Tree:" << endl;
		printCST(CST, 0); // output the CST
	}
}

// UTILITY FUNCTIONS
	
// function for returning the head of a token queue by popping it off; the queue then becomes its tail
// que		: the token queue to return the head of
// returns	: the head of the queue
Token Parser::hpop(queue<Token>& que)
{
	Token tok = que.front(); // save the head in a variable
	que.pop(); // remove the head
	return tok; // return the saved head
}

// function to make a node
// name		: name of this node
// returns	: a node
Node Parser::nmake(string name)
{
	queue<Node>* children = new queue<Node>;
	Node n = {name,"void", 0, children}; // initialize with type void and scope 0 to be changed during AST creation
	return n;
}

// prints out the concrete syntax tree
// n		: the current node
// level	: the depth of this node
void Parser::printCST(Node n, int level)
{
	for(int i=0; i < level; ++i) // for the node's depth
		cout << "-"; // print out a corresponding number of dashes
	cout << n.name << endl; // print node's name
	queue<Node> children = *n.children; // get the node's children
	while(!children.empty()) // for each child node
	{
		printCST(children.front(), level+1); // recurse
		children.pop();
	}
}


// PARSE FUNCTIONS

bool Parser::parseProgram(queue<Token>& que)
{
	// cout << "parseProgram" << endl;
	return 
		parseBlock(que, *CST.children) && 
		matchT_EOF(hpop(que), *CST.children);
}

bool Parser::parseBlock(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseBlock" << endl;
	Node n = nmake("<Block>");
	queue<Token> savedQue = que; // save queue for reverting since epsilon is involved
	if(matchT_OPEN_BRACE(hpop(que), *n.children) && 
		parseStatementList(que, *n.children) && 
		matchT_CLOSE_BRACE(hpop(que), *n.children)
		)
	{
		nodes.push(n);
		return true;
	}
	que = savedQue;
	return false;	
}

bool Parser::parseStatementList(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parsStmtLst" << endl;
	Node n = nmake("<StatementList>");
	queue<Token> savedQue = que; // save queue for reverting since epsilon is involved
	if(parseStatement(que, *n.children))
	{
		if(parseStatementList(que, *n.children))
		{
			nodes.push(n);
			return true;
		}
	}
	else
	{
		// epsilon
		que = savedQue;
		Node e = nmake("[epsilon]");
		queue<Node>& children = *n.children;
		children.push(e);
		nodes.push(n);
		return true;
	}
	return false; // never reached, but makes my C++ compiler happy
}

bool Parser::parseStatement(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseStmt" << endl;
	Node n = nmake("<Statement>");
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (parsePrintStatement(que, *n.children))
	{		
		nodes.push(n);
		return true; 
	}
	else 
		que = savedQue;
	if (parseAssignmentStatement(que, *n.children)) 
	{		
		nodes.push(n);
		return true; 
	} 
	else 
		que = savedQue;
	if (parseIfStatement(que, *n.children)) 
	{		
		nodes.push(n);
		return true; 
	}
	else 
		que = savedQue;
	if (parseWhileStatement(que, *n.children)) 
	{		
		nodes.push(n);
		return true; 
	} 
	else 
		que = savedQue;
	if (parseVarDecl(que, *n.children)) 
	{		
		nodes.push(n);
		return true; 
	} 
	else 
		que = savedQue;
	if (parseBlock(que, *n.children)) 
	{		
		nodes.push(n);
		return true; 
	} 
	else 
		que = savedQue;
	return false;
}

bool Parser::parsePrintStatement(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parsePrintStmt" << que.front().name << endl;
	Node n = nmake("<PrintStatement>");
	queue<Token> savedQue = que; // save queue for reverting
	if (matchT_PRINT(hpop(que), *n.children))
	{
		if(matchT_OPEN_PAREN(hpop(que), *n.children) && parseExpression(que, *n.children) && matchT_CLOSE_PAREN(hpop(que), *n.children))
		{		
			nodes.push(n);
			return true;
		}
		else
		{
			stmtError = error; // save whatever error one of the three conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	}
	que = savedQue;
	return false;
}

bool Parser::parseAssignmentStatement(queue<Token>& que, queue<Node>& nodes)
{ 
	// cout << "parseAssignStmt" << endl;
	Node n = nmake("<AssignmentStatement>");
	queue<Token> savedQue = que; // save queue for reverting
	if(matchT_ID(hpop(que), *n.children))
		if(matchT_ASSIGN(hpop(que), *n.children) && parseExpression(que, *n.children))
		{	
			nodes.push(n);
			return true;
		}
		else
		{
			stmtError = error; // save whatever error one of the two conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseVarDecl(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseVarDecl" << endl;
	Node n = nmake("<VarDecl>");
	queue<Token> savedQue = que; // save queue for reverting
	if(parseType(que, *n.children))
		if(matchT_ID(hpop(que), *n.children))
		{
			nodes.push(n);
			return true;
		}
		else
		{
			stmtError = error; // save the error the conditional returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseWhileStatement(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseWhileStmt" << endl;
	Node n = nmake("<WhileStatement>");
	queue<Token> savedQue = que; // save queue for reverting
	if(matchT_WHILE(hpop(que), *n.children))
		if(parseBooleanExpression(que, *n.children) && parseBlock(que, *n.children))
		{
			nodes.push(n);
			return true;
		}
		else
		{
			stmtError = error; // save whatever error one of the two conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseIfStatement(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseIfStmt" << endl;
	Node n = nmake("<IfStatement>");
	queue<Token> savedQue = que; // save queue for reverting
	if(matchT_IF(hpop(que), *n.children))
		if(parseBooleanExpression(que, *n.children) && parseBlock(que, *n.children))
		{
			nodes.push(n);
			return true;
		}
		else
		{
			stmtError = error; // save whatever error one the two conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseType(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseType" << endl;
	Node n = nmake("<type>");
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (matchT_INT(hpop(que), *n.children))
	{ 
		nodes.push(n);
		return true;
	} 
	else que = savedQue;
	if (matchT_STRING(hpop(que), *n.children))
	{ 
		nodes.push(n);
		return true;
	} 
	else que = savedQue;
	if (matchT_BOOLEAN(hpop(que), *n.children))
	{ 
		nodes.push(n);
		return true;
	} 
	else que = savedQue;
	return false;
}

bool Parser::parseExpression(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseExpr" << endl;
	Node n = nmake("<Expr>");
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (matchT_ID(hpop(que), *n.children))
	{	
		nodes.push(n);
		return true; 
	}
	else que = savedQue;
	if(parseStringExpression(que, *n.children) ||
		parseBooleanExpression(que, *n.children) ||
		parseIntExpression(que, *n.children))
	{
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::parseIntExpression(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseIntExpr" << endl;
	Node n = nmake("<IntExpr>");
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	
	if(matchT_DIGIT(hpop(que), *n.children))
	{
		savedQue = que;
		if(matchT_PLUS(hpop(que), *n.children) &&
			parseExpression(que, *n.children))
		{
			nodes.push(n);
			return true;
		}
		else
		{
			que = savedQue;
			nodes.push(n);
			return true;
		}
	}
	que = savedQue;
	return false;
}

bool Parser::parseStringExpression(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseStringExpr" << endl;
	Node n = nmake("<StringExpr>");
	queue<Token> savedQue = que; // save queue for reverting
	if( matchT_QUOTE(hpop(que), *n.children) &&
		parseCharList(que, *n.children) &&
		matchT_QUOTE(hpop(que), *n.children)
	  )
	{
		nodes.push(n);
		return true;
	}
	que = savedQue;
	return false;
}

bool Parser::parseBooleanExpression(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseBoolExpr" << endl;
	Node n = nmake("<BooleanExpr>");
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if(matchT_OPEN_PAREN(hpop(que), *n.children) &&
		parseExpression(que, *n.children) &&
		parseBoolOp(que, *n.children) &&
		parseExpression(que, *n.children) &&
		matchT_CLOSE_PAREN(hpop(que), *n.children))
	{		
		nodes.push(n);
		return true;
	}
	else
	{
		que = savedQue;
		if(parseBoolval(que, *n.children))
		{
			nodes.push(n);
			return true;
		}
		return false;
	}
}

bool Parser::parseBoolOp(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseBoolOp" << endl;
	Node n = nmake("<boolop>");
	queue<Token> savedQue = que; // save queue for reverting
	Token tok = hpop(que); // only two paths here, each taking just the head token
	if(matchT_EQUALS(tok, *n.children) ||
		matchT_NOT_EQUALS(tok, *n.children))
	{
		nodes.push(n);
		return true;
	}
	que = savedQue;
	return false;
}

bool Parser::parseCharList(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseCharList" << endl;
	Node n = nmake("<CharList>");
	charList = true; // we are now inside a charList
	queue<Token> savedQue = que; // save queue for reverting since epsilon is involved
	Token tok = hpop(que); // get the head token
	if(matchT_SPACE(tok, nodes) || 
		matchT_ID(tok, nodes) ||
		matchT_FALSE(tok, nodes) ||
		matchT_TRUE(tok, nodes) ||
		matchT_WHILE(tok, nodes) ||
		matchT_PRINT(tok, nodes) ||
		matchT_STRING(tok, nodes) ||
		matchT_BOOLEAN(tok, nodes) ||
		matchT_INT(tok, nodes) ||
		matchT_IF(tok, nodes)) // space, char, or any keyword comprised only of characters
	{
		//nodes.push(n);
		parseCharList(que, nodes);
	}
	else
	{
		//epsilon
		que = savedQue;
		charList = false; // exiting the charList
		// convert characters and other valid keywords into a charList
		queue<Node>& children = *n.children;
		string s = "[";
		Node saved = nodes.front(); // save ["] node
		nodes.pop(); // get rid of ["] node
		while(!nodes.empty())
		{
			Node head = nodes.front();
			if(head.name == "[space]") s.append(" "); // convert [space] to actual space
			else s.append(head.name.substr(1, head.name.length()-2)); // otherwise trim off [ and ]
			nodes.pop();
		}
		s.append("]");
		nodes.push(saved); // put ["] back
		children.push(nmake(s));
		nodes.push(n); // push converted charList 
	}
	return true;
}

bool Parser::parseBoolval(queue<Token>& que, queue<Node>& nodes)
{
	// cout << "parseBoolVal" << endl;
	Node n = nmake("<boolval>");
	queue<Token> savedQue = que; // save queue for reverting
	Token tok = hpop(que); // only two paths here, each taking just the head token
	if(matchT_TRUE(tok, *n.children) ||
		matchT_FALSE(tok, *n.children))
	{
		nodes.push(n);
		return true;
	}
	que = savedQue;
	return false;
}

// MATCH FUNCTIONS

bool Parser::matchT_DIGIT(Token tok, queue<Node>& nodes)
{
	// cout << "	match digit " << tok.value << endl;
	Node n = nmake("["+tok.value+"]");
	n.type = "digit";
	error = "[" + tok.value + "] is not a valid digit. Valid digits include natural numbers [0-9].";
	errorLine = tok.lineNum;
	if(tok.name == "T_DIGIT")
	{
		if(verbose) cout << "Matched a T_DIGIT." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_ID(Token tok, queue<Node>& nodes)
{
	// cout << "	match id " << tok.value << endl;
	Node n = nmake("["+tok.value+"]");
	n.type = "id";
	if(!charList ) 
		error = "[" + tok.value + "] is not a valid identifier. Valid identifiers include lowercase letters [a-z].";
	else 
		error = "[" + tok.value + "] is not a valid character. Characters can only be lowercase letters [a-z] or the space character [ ].";
	errorLine = tok.lineNum;
	if(tok.name == "T_ID")
	{
		if(verbose) cout << "Matched a T_ID." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_EOF(Token tok, queue<Node>& nodes)
{
	// cout << "	match eof " << tok.value << endl;
	Node n = nmake("[$]");
	error = "Program cannot end with [" + tok.value + "]. Programs may only end with [$].";
	errorLine = tok.lineNum;
	if(tok.name == "T_EOF")
	{
		if(verbose) cout << "Matched a T_EOF." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_PLUS(Token tok, queue<Node>& nodes)
{
	// cout << "	match plus " << tok.value << endl;
	// no error message here isnce an integer expression can also be just a digit, which is checked second
	Node n = nmake("[+]");
	if(tok.name == "T_PLUS")
	{
		if(verbose) cout << "Matched a T_PLUS." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_ASSIGN(Token tok, queue<Node>& nodes)
{
	// cout << "	match assign " << tok.value << endl;
	Node n = nmake("[=]");
	error = "Expecting the assignment operator [=]. Instead found the token [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_ASSIGN")
	{
		if(verbose) cout << "Matched a T_ASSIGN." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_OPEN_BRACE(Token tok, queue<Node>& nodes)
{
	// cout << "	match open brace " << tok.value << endl;
	Node n = nmake("[{]");
	error = "Expecting an open brace [{] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_OPEN_BRACE")
	{
		if(verbose) cout << "Matched a T_OPEN_BRACE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_CLOSE_BRACE(Token tok, queue<Node>& nodes)
{
	// cout << "	match close brace " << tok.value << endl;
	Node n = nmake("[}]");
	error = "Expecting a closing brace [}] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_CLOSE_BRACE")
	{
		if(verbose) cout << "Matched a T_CLOSE_BRACE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_OPEN_PAREN(Token tok, queue<Node>& nodes)
{
	// cout << "	match open paren " << tok.value << endl;
	Node n = nmake("[(]");
	error = "Expecting an open parenthesis [(] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_OPEN_PAREN")
	{
		if(verbose) cout << "Matched a T_OPEN_PAREN." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_CLOSE_PAREN(Token tok, queue<Node>& nodes)
{
	// cout << "	match close paren " << tok.value << endl;
	Node n = nmake("[)]");
	error = "Expecting a closing parenthesis [)] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_CLOSE_PAREN")
	{
		if(verbose) cout << "Matched a T_CLOSE_PAREN." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_QUOTE(Token tok, queue<Node>& nodes)
{
	// cout << "	match quote " << tok.value << endl;
	Node n = nmake("[\"]");
	error = "Strings must be wrapped in quotation marks. Expecting a quote [\"] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_QUOTE")
	{
		if(verbose) cout << "Matched a T_QUOTE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_EQUALS(Token tok, queue<Node>& nodes)
{
	// cout << "	match equals " << tok.value << endl;
	Node n = nmake("[==]");
	error = "[" + tok.value + "] is not a valid boolean operator. Valid boolean operators include [==] and [!=].";
	errorLine = tok.lineNum;
	if(tok.name == "T_EQUALS")
	{
		if(verbose) cout << "Matched a T_EQUALS." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_NOT_EQUALS(Token tok, queue<Node>& nodes)
{
	// cout << "	match not equals " << tok.value << endl;
	Node n = nmake("[!=]");
	error = "[" + tok.value + "] is not a valid boolean operator. Valid boolean operators include [==] and [!=].";
	errorLine = tok.lineNum;
	if(tok.name == "T_NOT_EQUALS")
	{
		if(verbose) cout << "Matched a T_NOT_EQUALS." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_FALSE(Token tok, queue<Node>& nodes)
{
	// cout << "	match false " << tok.value << endl;
	Node n = nmake("[false]");
	error = "[" + tok.value + "] is not a valid boolean value. Valid boolean values include [true] and [false].";
	errorLine = tok.lineNum;
	if(tok.name == "T_FALSE")
	{
		if(verbose) cout << "Matched a T_FALSE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_TRUE(Token tok, queue<Node>& nodes)
{
	// cout << "	match true " << tok.value << endl;
	Node n = nmake("[true]");
	error = "[" + tok.value + "] is not a valid boolean value. Valid boolean values include [true] and [false].";
	errorLine = tok.lineNum;
	if(tok.name == "T_TRUE")
	{
		if(verbose) cout << "Matched a T_TRUE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_WHILE(Token tok, queue<Node>& nodes)
{
	// cout << "	match while " << tok.value << endl;
	Node n = nmake("[while]");
	error = "Expecting the [while] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_WHILE")
	{
		if(verbose) cout << "Matched a T_WHILE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_PRINT(Token tok, queue<Node>& nodes)
{
	// cout << "	match print " << tok.value << endl;
	Node n = nmake("[print]");
	error = "Expecting the [print] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_PRINT")
	{
		if(verbose) cout << "Matched a T_PRINT." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_STRING(Token tok, queue<Node>& nodes)
{
	// cout << "	match string " << tok.value << endl;
	Node n = nmake("[string]");
	error = "Expecting the [string] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_STRING")
	{
		if(verbose) cout << "Matched a T_STRING." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_BOOLEAN(Token tok, queue<Node>& nodes)
{
	// cout << "	match bool " << tok.value << endl;
	Node n = nmake("[boolean]");
	error = "Expecting the [boolean] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_BOOLEAN")
	{
		if(verbose) cout << "Matched a T_BOOLEAN." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_INT(Token tok, queue<Node>& nodes)
{
	// cout << "	match int " << tok.value << endl;
	Node n = nmake("[int]");
	error = "Expecting the [int] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_INT")
	{
		if(verbose) cout << "Matched a T_INT." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_IF(Token tok, queue<Node>& nodes)
{
	// cout << "	match if " << tok.value << endl;
	Node n = nmake("[if]");
	error = "Expecting keyword [if] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	if(tok.name == "T_IF")
	{
		if(verbose) cout << "Matched a T_IF." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}

bool Parser::matchT_SPACE(Token tok, queue<Node>& nodes)
{
	// cout << "	match space " << tok.value << endl;
	Node n = nmake("[space]");
	error = "[" + tok.value + "] is not a valid character. Characters can only be lowercase letters [a-z] or the space character [ ].";
	errorLine = tok.lineNum;
	if(tok.name == "T_SPACE")
	{
		if(verbose) cout << "Matched a T_SPACE." << endl;
		nodes.push(n);
		return true;
	}
	return false;
}