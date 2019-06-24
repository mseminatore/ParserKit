#include <iostream>
#include <map>
#include <stack>
#include "tableparser.h"

//
int TableParser::yyparse()
{
	int token, rule;

	initTable();

	token = yylex();

	while (ss.size() > 0)
	{
		if (token == ss.top())
		{
			if (token > 0 && token < 256)
				printf("Matched symbol: '%c'\n", token);
			else
				printf("Matched symbol: %d\n", token);

			ss.pop();
			if (ss.size() > 0)
				token = yylex();
		}
		else
		{
			rule = table[ss.top()][token];
			printf("Rule %d\n", rule);

			// if there was a rule parse error
			if (!yyrule(rule))
				break;
		}
	}

	return 0;
}

//
void TableParser::yyerror(const std::string &str)
{
	printf("error: %s\n", str.c_str());
}

//
void TableParser::yywarning(const std::string &str)
{
	printf("warning: %s\n", str.c_str());
}
