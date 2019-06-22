#pragma once

class TableParser
{
protected:
	using Symbols = int;
	using Rule = int;

	map< Symbols, map<Symbols, Rule> > table;
	stack<Symbols> ss;	// symbol stack

public:
	~TableParser() = default;

	virtual int yyparse();
	
	virtual void yyerror(std::string &str);
	virtual void yywarning(std::string &str);

	virtual void initTable() = 0;
	virtual void yyrule() = 0;
};
