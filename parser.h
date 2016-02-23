using namespace std;
using std::string;
using std::queue;

// the Parser class object definition
class Parser
{
	// public class access
	public:
		Parser(queue<Token>&); // constructor
	// private class access
	private:
		string error; // the error message to return
		queue<Token> tpop(queue<Token>&);
		Token hpop(queue<Token>&);
		bool parseProgram(queue<Token>&);
		bool parseBlock(queue<Token> &);
		bool parseStatementList(queue<Token>&);
		bool parseStatement(queue<Token>&);
		bool parsePrintStatement(queue<Token>&);
		bool parseAssignmentStatement(queue<Token>&);
		bool parseVarDecl(queue<Token>&);
		bool parseWhileStatement(queue<Token>&);
		bool parseIfStatement(queue<Token>&);
		bool parseType(queue<Token>&);
		bool parseExpression(queue<Token>&);
		bool parseIntExpression(queue<Token>&);
		bool parseStringExpression(queue<Token>&);
		bool parseBooleanExpression(queue<Token>&);
		bool parseBoolOp(queue<Token>&);
		bool parseCharList(queue<Token>&);
		bool parseBoolval(queue<Token>&);
		bool matchT_EOF(Token);
		bool matchT_PLUS(Token);
		bool matchT_OPEN_BRACE(Token);
		bool matchT_CLOSE_BRACE(Token);
		bool matchT_OPEN_PAREN(Token);
		bool matchT_CLOSE_PAREN(Token);
		bool matchT_QUOTE(Token);
		bool matchT_EQUALS(Token);
		bool matchT_NOT_EQUALS(Token);
		bool matchT_FALSE(Token);
		bool matchT_TRUE(Token);
		bool matchT_WHILE(Token);
		bool matchT_PRINT(Token);
		bool matchT_ID(Token);
		bool matchT_STRING(Token);
		bool matchT_BOOLEAN(Token);
		bool matchT_IF(Token);
		bool matchT_SPACE(Token);
		bool matchT_ASSIGN(Token);
		bool matchT_DIGIT(Token);
		bool matchT_INT(Token);
};

// the Parser constructor
// que	: reference to the token queue the Parser should use
Parser::Parser(queue<Token>& que)
{
	error = ""; // set error to nothing
	if(parseProgram(que))
		cout << "Praise the sun!" << endl;
	else
		cout << "Found an error!" << endl;
}

// UTILITY FUNCTIONS
/*
// function for returning the tail of a token queue by popping off the head
// que		: the token queue to return the tail of
// returns	: the tail of the queue
queue<Token> Parser::tpop(queue<Token>& que)
{
	que.pop(); // remove the head
	return que; // return the queue
}
*/
		
// function for returning the head of a token queue by popping it off; the queue then becomes its tail
// que		: the token queue to return the head of
// returns	: the head of the queue
Token Parser::hpop(queue<Token>& que)
{
	Token tok = que.front(); // save the head in a variable
	que.pop(); // remove the head
	return tok; // return the saved head
}

// PARSE FUNCTIONS

bool Parser::parseProgram(queue<Token>& que)
{
	return 
		parseBlock(que) && 
		matchT_EOF(hpop(que));
}

bool Parser::parseBlock(queue<Token>& que)
{
	return 
		matchT_OPEN_BRACE(hpop(que)) && 
		parseStatementList(que) && 
		matchT_CLOSE_BRACE(hpop(que));
}

bool Parser::parseStatementList(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since epsilon is involved
	if(parseStatement(que))
		return parseStatementList(que);
	else
	{
		que = savedQue;
		return true; // epsilon
	}
}

bool Parser::parseStatement(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (parsePrintStatement(que)) return true; else que = savedQue;
	if (parseAssignmentStatement(que)) return true; else que = savedQue;
	if (parseIfStatement(que)) return true; else que = savedQue;
	if (parseWhileStatement(que)) return true; else que = savedQue;
	if (parseVarDecl(que)) return true; else que = savedQue;
	if (parseBlock(que)) return true; else que = savedQue;
	return false;
}

bool Parser::parsePrintStatement(queue<Token>& que)
{
	return 
		matchT_PRINT(hpop(que)) &&
		matchT_OPEN_PAREN(hpop(que)) &&
		parseExpression(que);
		matchT_CLOSE_PAREN(hpop(que));
}

bool Parser::parseAssignmentStatement(queue<Token>& que)
{
	return 
		matchT_ID(hpop(que)) &&
		matchT_ASSIGN(hpop(que)) &&
		parseExpression(que);
}

bool Parser::parseVarDecl(queue<Token>& que)
{
	return
		parseType(que) &&
		matchT_ID(hpop(que));
}

bool Parser::parseWhileStatement(queue<Token>& que)
{
	return
		matchT_WHILE(hpop(que)) &&
		parseBooleanExpression(que) &&
		parseBlock(que);
}

bool Parser::parseIfStatement(queue<Token>& que)
{
	return 
		matchT_IF(hpop(que)) &&
		parseBooleanExpression(que) &&
		parseBlock(que);
}

bool Parser::parseType(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (matchT_INT(hpop(que))) return true; else que = savedQue;
	if (matchT_STRING(hpop(que))) return true; else que = savedQue;
	if (matchT_BOOLEAN(hpop(que))) return true; else que = savedQue;
	return false;
}

bool Parser::parseExpression(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (matchT_ID(hpop(que))) return true; else que = savedQue;
	return
		parseStringExpression(que) ||
		parseBooleanExpression(que) ||
		parseIntExpression(que);
}

bool Parser::parseIntExpression(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if(matchT_DIGIT(hpop(que)) &&
		matchT_PLUS(hpop(que)) &&
		parseExpression(que))
		return true;
	else
	{
		que = savedQue;
		matchT_DIGIT(hpop(que));
	}
}

bool Parser::parseStringExpression(queue<Token>& que)
{
	return
		matchT_QUOTE(hpop(que)) &&
		parseCharList(que) &&
		matchT_QUOTE(hpop(que));
}

bool Parser::parseBooleanExpression(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if(matchT_OPEN_PAREN(hpop(que)) &&
		parseExpression(que) &&
		parseBoolOp(que))
		return true;
	else
	{
		que = savedQue;
		parseBoolval(que);
	}
}

bool Parser::parseBoolOp(queue<Token>& que)
{
	Token tok = hpop(que); // only two paths here, each taking just the head token
	return
		matchT_EQUALS(tok) ||
		matchT_NOT_EQUALS(tok);
}

bool Parser::parseCharList(queue<Token>& que)
{
	queue<Token> savedQue = que; // save queue for reverting since epsilon is involved
	Token tok = hpop(que); // only two paths here, each taking just the head token
	if(matchT_SPACE(tok) || matchT_ID(tok)) // space or char
		return parseCharList(que);
	else
	{
		que = savedQue;
		return true; //epsilon
	}
}

bool Parser::parseBoolval(queue<Token>& que)
{
	Token tok = hpop(que); // only two paths here, each taking just the head token
	return
		matchT_TRUE(tok) ||
		matchT_FALSE(tok);
}

// MATCH FUNCTIONS

bool Parser::matchT_DIGIT(Token tok)
{
	return tok.name == "T_DIGIT";
}

bool Parser::matchT_ID(Token tok)
{
	return tok.name == "T_ID";
}

bool Parser::matchT_EOF(Token tok)
{
	return tok.name == "T_EOF";
}

bool Parser::matchT_PLUS(Token tok)
{
	return tok.name == "T_PLUS";
}

bool Parser::matchT_ASSIGN(Token tok)
{
	return tok.name == "T_ASSIGN";
}

bool Parser::matchT_OPEN_BRACE(Token tok)
{
	return tok.name == "T_OPEN_BRACE";
}

bool Parser::matchT_CLOSE_BRACE(Token tok)
{
	return tok.name == "T_CLOSE_BRACE";
}

bool Parser::matchT_OPEN_PAREN(Token tok)
{
	return tok.name == "T_OPEN_PAREN";
}

bool Parser::matchT_CLOSE_PAREN(Token tok)
{
	return tok.name == "T_CLOSE_PAREN";
}

bool Parser::matchT_QUOTE(Token tok)
{
	return tok.name == "T_QUOTE";
}

bool Parser::matchT_EQUALS(Token tok)
{
	return tok.name == "T_EQUALS";
}

bool Parser::matchT_NOT_EQUALS(Token tok)
{
	return tok.name == "T_NOT_EQUALS";
}

bool Parser::matchT_FALSE(Token tok)
{
	return tok.name == "T_FALSE";
}

bool Parser::matchT_TRUE(Token tok)
{
	return tok.name == "T_TRUE";
}

bool Parser::matchT_WHILE(Token tok)
{
	return tok.name == "T_WHILE";
}

bool Parser::matchT_PRINT(Token tok)
{
	return tok.name == "T_PRINT";
}

bool Parser::matchT_STRING(Token tok)
{
	return tok.name == "T_STRING";
}

bool Parser::matchT_BOOLEAN(Token tok)
{
	return tok.name == "T_BOOLEAN";
}

bool Parser::matchT_INT(Token tok)
{
	return tok.name == "T_INT";
}

bool Parser::matchT_IF(Token tok)
{
	return tok.name == "T_IF";
}

bool Parser::matchT_SPACE(Token tok)
{
	return tok.name == "T_SPACE";
}