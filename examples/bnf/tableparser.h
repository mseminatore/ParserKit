#pragma once

#include <map>
#include <stack>

#define YYLOG stdout
#define YYBUFSIZE 2048

class TableParser
{
protected:
	using Symbols = int;
	using Rule = int;
	using LexicalAnalyzer = int(*)();

	LexicalAnalyzer yylex;

	// parsing table
	std::map< Symbols, std::map<Symbols, Rule> > table;
	
	std::map<Symbols, Rule> actions;

	// symbol stack
	std::stack<Symbols> ss;

	bool yydebug = false;

public:
	TableParser(LexicalAnalyzer lexer) { yylex = lexer; };
	~TableParser() = default;

	void setDebug(bool onoff) {
		yydebug = onoff;
	}

	virtual int yyparse();
	
	virtual void yyerror(const std::string &str);
	virtual void yywarning(const std::string &str);
	virtual void yylog(const char *fmt, ...);

	virtual void initTable() = 0;
	virtual int yyrule(int rule) = 0;
	virtual int yyaction(int action) = 0;

	virtual void tokenMatch(int token) = 0;
	virtual void ruleMatch(int rule) = 0;
};
