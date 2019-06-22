#include <iostream>
#include <map>
#include <stack>
#include "tableparser.h"

//
int TableParser::yyparse()
{
	int token, rule;

	initTable();

	while (ss.size() > 0)
	{
		token = yylex();
		if (token == ss.top())
		{
			printf("Matched symbols: %s\n", token);
			ss.pop();
		}
		else
		{
			rule = table[ss.top()][token];
			printf("Rule %d\n", rule);

			yyrule(rule);
		}
	}

	return 0;
}

//
void TableParser::yyerror(std::string &str)
{

}

//
void TableParser::yywarning(std::string &str)
{

}
