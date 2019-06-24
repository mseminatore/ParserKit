#pragma once

#include <map>
#include <stack>

class TableParser
{
protected:
	using Symbols = int;
	using Rule = int;

	std::map< Symbols, std::map<Symbols, Rule> > table;
	std::stack<Symbols> ss;	// symbol stack

public:
	TableParser() {};
	~TableParser() = default;

	virtual int yyparse();
	
	virtual void yyerror(const std::string &str);
	virtual void yywarning(const std::string &str);

	virtual void initTable() = 0;
	virtual int yyrule(int rule) = 0;
	virtual int yylex() = 0;
};
