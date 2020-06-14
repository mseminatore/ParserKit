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
				yylog("\nMatched character: '%c'\n", token);
			else
				yylog("\nMatched token: %d\n", token);

			// TODO - what do we push for a non-terminal symbol? yylval is not populated with data!
			// save the yylval on the value stack
			if (token)
				tokenMatch(token);

			// pop the matched symbol from the stack
			ss.pop();

			// if there is more parsing to do, then get the next token
			if (ss.size() > 0)
				token = yylex();
		}
		else
		{
			// perform any actions if they exist
			if (yyaction(ss.top()))
			{
//				yylog("Action %d\n", ss.top());

				// pop the executed action off the stack
				ss.pop();
				
				// there might be more actions?
				continue;
			}

			rule = table[ss.top()][token];
			yylog("\nPredict rule %d: ", rule);

			// process the rule
			yyrule(rule);
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