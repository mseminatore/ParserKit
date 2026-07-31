// calc.cpp — driver for the standalone "calc" example
//
// Usage:
//   calc                 REPL: evaluate one expression per line from stdin
//   calc <file.calc>     Evaluate every expression in a file
//   calc [...] -v        Verbose: also trace parser rule calls

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "calcparser.h"

int main(int argc, char *argv[])
{
	bool verbose = false;
	const char *file = nullptr;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-v") == 0)
			verbose = true;
		else
			file = argv[i];
	}

	CalcParser parser;
	parser.yydebug = verbose;

	if (file)
	{
		parser.parseFile(file);
	}
	else
	{
		// REPL: each line is parsed independently via parseData()
		char line[256];

		printf("calc> ");
		while (fgets(line, sizeof(line), stdin))
		{
			parser.parseData(line, "<stdin>", nullptr);
			printf("calc> ");
		}
	}

	if (parser.getErrorCount() > 0)
	{
		fprintf(stderr, "%u error(s) found.\n", parser.getErrorCount());
		return 1;
	}

	return 0;
}
