using namespace std;
using std::string;
using std::queue;

// the Parser class object definition
class Parser
{
	// public class access
	public:
		Parser(queue<Token>&); // constructor
		int numErrors; // // number of parser errors
	// private class access
	private:
		string error; // the error message to return
		string stmtError; // error message for incorrect statement, since there are many different types of statements
		int errorLine; // line number of the error
		int stmtErrorLine; // line number of statement error
		bool charList; // true if we are inside a charList
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
	numErrors = 0; // no errors at the start
	error = ""; // set error to nothing
	stmtError= ""; // set statement error to nothing
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

// PARSE FUNCTIONS

bool Parser::parseProgram(queue<Token>& que)
{
	// cout << "parseProgram" << endl;
	return 
		parseBlock(que) && 
		matchT_EOF(hpop(que));
}

bool Parser::parseBlock(queue<Token>& que)
{
	// cout << "parseBlock" << endl;
	return 
		matchT_OPEN_BRACE(hpop(que)) && 
		parseStatementList(que) && 
		matchT_CLOSE_BRACE(hpop(que));
}

bool Parser::parseStatementList(queue<Token>& que)
{
	// cout << "parsStmtLst" << endl;
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
	// cout << "parseStmt" << endl;
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
	// cout << "parsePrintStmt" << que.front().name << endl;
	queue<Token> savedQue = que; // save queue for reverting
	if (matchT_PRINT(hpop(que)))
		if(matchT_OPEN_PAREN(hpop(que)) && parseExpression(que) && matchT_CLOSE_PAREN(hpop(que)))
			return true;
		else
		{
			stmtError = error; // save whatever error one of the three conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseAssignmentStatement(queue<Token>& que)
{ 
	// cout << "parseAssignStmt" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	if(matchT_ID(hpop(que)))
		if(matchT_ASSIGN(hpop(que)) && parseExpression(que))
			return true;
		else
		{
			stmtError = error; // save whatever error one of the two conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseVarDecl(queue<Token>& que)
{
	// cout << "parseVarDecl" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	if(parseType(que))
		if(matchT_ID(hpop(que)))
			return true;
		else
		{
			stmtError = error; // save the error the conditional returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseWhileStatement(queue<Token>& que)
{
	// cout << "parseWhileStmt" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	if(matchT_WHILE(hpop(que)))
		if(parseBooleanExpression(que) && parseBlock(que))
			return true;
		else
		{
			stmtError = error; // save whatever error one of the two conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseIfStatement(queue<Token>& que)
{
	// cout << "parseIfStmt" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	if(matchT_IF(hpop(que)))
		if(parseBooleanExpression(que) && parseBlock(que))
			return true;
		else
		{
			stmtError = error; // save whatever error one the two conditionals returned
			stmtErrorLine = errorLine; // save the line number of the error
		}
	que = savedQue;
	return false;
}

bool Parser::parseType(queue<Token>& que)
{
	// cout << "parseType" << endl;
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (matchT_INT(hpop(que))) return true; else que = savedQue;
	if (matchT_STRING(hpop(que))) return true; else que = savedQue;
	if (matchT_BOOLEAN(hpop(que))) return true; else que = savedQue;
	return false;
}

bool Parser::parseExpression(queue<Token>& que)
{
	// cout << "parseExpr" << endl;
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if (matchT_ID(hpop(que))) return true; else que = savedQue;
	return
		parseStringExpression(que) ||
		parseBooleanExpression(que) ||
		parseIntExpression(que);
}

bool Parser::parseIntExpression(queue<Token>& que)
{
	// cout << "parseIntExpr" << endl;
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if(matchT_DIGIT(hpop(que)) &&
		matchT_PLUS(hpop(que)) &&
		parseExpression(que))
		return true;
	else
	{
		que = savedQue;
		return matchT_DIGIT(hpop(que));
	}
}

bool Parser::parseStringExpression(queue<Token>& que)
{
	// cout << "parseStringExpr" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	if( matchT_QUOTE(hpop(que)) &&
		parseCharList(que) &&
		matchT_QUOTE(hpop(que))
	  )
		return true;
	que = savedQue;
	return false;
}

bool Parser::parseBooleanExpression(queue<Token>& que)
{
	// cout << "parseBoolExpr" << endl;
	queue<Token> savedQue = que; // save queue for reverting since multiple paths are involved
	if(matchT_OPEN_PAREN(hpop(que)) &&
		parseExpression(que) &&
		parseBoolOp(que) &&
		parseExpression(que) &&
		matchT_CLOSE_PAREN(hpop(que))) 
		return true;
	else
	{
		que = savedQue;
		return parseBoolval(que);
	}
}

bool Parser::parseBoolOp(queue<Token>& que)
{
	// cout << "parseBoolOp" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	Token tok = hpop(que); // only two paths here, each taking just the head token
	if(matchT_EQUALS(tok) ||
		matchT_NOT_EQUALS(tok))
		return true;
	que = savedQue;
	return false;
}

bool Parser::parseCharList(queue<Token>& que)
{
	// cout << "parseCharList" << endl;
	charList = true; // we are now inside a charList
	queue<Token> savedQue = que; // save queue for reverting since epsilon is involved
	Token tok = hpop(que); // only two paths here, each taking just the head token
	if(matchT_SPACE(tok) || matchT_ID(tok)) // space or char
		parseCharList(que);
	else
	{
		que = savedQue;
		//epsilon
	}
	charList = false; // exiting the charList
	return true;
}

bool Parser::parseBoolval(queue<Token>& que)
{
	// cout << "parseBoolVal" << endl;
	queue<Token> savedQue = que; // save queue for reverting
	Token tok = hpop(que); // only two paths here, each taking just the head token
	if(matchT_TRUE(tok) ||
		matchT_FALSE(tok))
		return true;
	que = savedQue;
	return false;
}

// MATCH FUNCTIONS

bool Parser::matchT_DIGIT(Token tok)
{
	// cout << "	match digit " << tok.value << endl;
	error = "[" + tok.value + "] is not a valid digit. Valid digits include natural numbers [0-9].";
	errorLine = tok.lineNum;
	return tok.name == "T_DIGIT";
}

bool Parser::matchT_ID(Token tok)
{
	// cout << "	match id " << tok.value << endl;
	if(!charList ) 
		error = "[" + tok.value + "] is not a valid identifier. Valid identifiers include lowercase letters [a-z].";
	else 
		error = "[" + tok.value + "] is not a valid character. Characters can only be lowercase letters [a-z] or the space character [ ].";
	errorLine = tok.lineNum;
	return tok.name == "T_ID";
}

bool Parser::matchT_EOF(Token tok)
{
	// cout << "	match eof " << tok.value << endl;
	error = "Program cannot end with [" + tok.value + "]. Programs may only end with [$].";
	errorLine = tok.lineNum;
	return tok.name == "T_EOF";
}

bool Parser::matchT_PLUS(Token tok)
{
	// cout << "	match plus " << tok.value << endl;
	// no error message here isnce an integer expression can also be just a digit, which is checked second
	return tok.name == "T_PLUS";
}

bool Parser::matchT_ASSIGN(Token tok)
{
	// cout << "	match assign " << tok.value << endl;
	error = "Expecting the assignment operator [=]. Instead found the token [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_ASSIGN";
}

bool Parser::matchT_OPEN_BRACE(Token tok)
{
	// cout << "	match open brace " << tok.value << endl;
	error = "Expecting an open brace [{] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_OPEN_BRACE";
}

bool Parser::matchT_CLOSE_BRACE(Token tok)
{
	// cout << "	match close brace " << tok.value << endl;
	error = "Expecting a closing brace [}] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_CLOSE_BRACE";
}

bool Parser::matchT_OPEN_PAREN(Token tok)
{
	// cout << "	match open paren " << tok.value << endl;
	error = "Expecting an open parenthesis [(] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_OPEN_PAREN";
}

bool Parser::matchT_CLOSE_PAREN(Token tok)
{
	// cout << "	match close paren " << tok.value << endl;
	error = "Expecting a closing parenthesis [)] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_CLOSE_PAREN";
}

bool Parser::matchT_QUOTE(Token tok)
{
	// cout << "	match quote " << tok.value << endl;
	error = "Strings must be wrapped in quotation marks. Expecting a quote [\"] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_QUOTE";
}

bool Parser::matchT_EQUALS(Token tok)
{
	// cout << "	match equals " << tok.value << endl;
	error = "[" + tok.value + "] is not a valid boolean operator. Valid boolean operators include [==] and [!=].";
	errorLine = tok.lineNum;
	return tok.name == "T_EQUALS";
}

bool Parser::matchT_NOT_EQUALS(Token tok)
{
	// cout << "	match not equals " << tok.value << endl;
	error = "[" + tok.value + "] is not a valid boolean operator. Valid boolean operators include [==] and [!=].";
	errorLine = tok.lineNum;
	return tok.name == "T_NOT_EQUALS";
}

bool Parser::matchT_FALSE(Token tok)
{
	// cout << "	match false " << tok.value << endl;
	error = "[" + tok.value + "] is not a valid boolean value. Valid boolean values include [true] and [false].";
	errorLine = tok.lineNum;
	return tok.name == "T_FALSE";
}

bool Parser::matchT_TRUE(Token tok)
{
	// cout << "	match true " << tok.value << endl;
	error = "[" + tok.value + "] is not a valid boolean value. Valid boolean values include [true] and [false].";
	errorLine = tok.lineNum;
	return tok.name == "T_TRUE";
}

bool Parser::matchT_WHILE(Token tok)
{
	// cout << "	match while " << tok.value << endl;
	error = "Expecting the [while] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_WHILE";
}

bool Parser::matchT_PRINT(Token tok)
{
	// cout << "	match print " << tok.value << endl;
	error = "Expecting the [print] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_PRINT";
}

bool Parser::matchT_STRING(Token tok)
{
	// cout << "	match string " << tok.value << endl;
	error = "Expecting the [string] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_STRING";
}

bool Parser::matchT_BOOLEAN(Token tok)
{
	// cout << "	match bool " << tok.value << endl;
	error = "Expecting the [boolean] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_BOOLEAN";
}

bool Parser::matchT_INT(Token tok)
{
	// cout << "	match int " << tok.value << endl;
	error = "Expecting the [int] keyword before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_INT";
}

bool Parser::matchT_IF(Token tok)
{
	// cout << "	match if " << tok.value << endl;
	error = "Expecting keyword [if] before the [" + tok.value + "].";
	errorLine = tok.lineNum;
	return tok.name == "T_IF";
}

bool Parser::matchT_SPACE(Token tok)
{
	// cout << "	match space " << tok.value << endl;
	error = "[" + tok.value + "] is not a valid character. Characters can only be lowercase letters [a-z] or the space character [ ].";
	errorLine = tok.lineNum;
	return tok.name == "T_SPACE";
}