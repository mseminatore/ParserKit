#include <stdarg.h>
#include <iostream>
#include <map>
#include <stack>
#include "tableparser.h"

//
int TableParser::yyparse()
{
	int token, rule;

	// setup the LL(1) parsing table
	initTable();

	token = yylex();

	while (ss.size() > 0)
	{
		// if the token matches TOS then we match the token
		if (token == ss.top())
		{
			if (token > 0 && token < 256)
				yylog("Matched symbol: '%c'\n", token);
			else
				yylog("Matched symbol: %d\n", token);

			// save the yylval on the value stack
			tokenMatch(token);

			ss.pop();

			// if there is more parsing to do, then get the next token
			if (ss.size() > 0)
				token = yylex();
		}
		else
		{
			rule = table[ss.top()][token];
			yylog("Rule %d\n", rule);

			// check if there was a rule parse error
			if (yyrule(rule))
				ruleMatch(rule);
			else
				break;
		}
	}

	return 0;
}

//
void TableParser::yyerror(const std::string &str)
{
	yylog("error: %s\n", str.c_str());
}

//
void TableParser::yywarning(const std::string &str)
{
	yylog("warning: %s\n", str.c_str());
}

//
void TableParser::yylog(const char *fmt, ...)
{
	if (!yydebug)
		return;

	char buf[YYBUFSIZE];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	fputs(buf, YYLOG);
}