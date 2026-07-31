// ini.cpp — driver for the standalone INI parser example
//
// Usage:
//   ini <file.ini>       Parse and print each section/key as it's read
//   ini <file.ini> -v    Verbose: also trace scope push()/pop() calls

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "iniparser.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: ini <file.ini> [-v]\n");
		return 1;
	}

	IniParser parser;

	if (argc > 2 && strcmp(argv[2], "-v") == 0)
		parser.yydebug = true;

	parser.parseFile(argv[1]);

	if (parser.getErrorCount() > 0)
	{
		fprintf(stderr, "%u error(s) found.\n", parser.getErrorCount());
		return 1;
	}

	return 0;
}
