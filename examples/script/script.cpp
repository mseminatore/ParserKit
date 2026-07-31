// script.cpp — driver for the standalone "script" language example
//
// Usage:
//   script <file.script> [-v]       Parse and run a script file
//   script -e "<script text>" [-v]  Parse and run an inline script

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <vector>
#include "scriptparser.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: script <file.script> [-v]\n");
		fprintf(stderr, "       script -e \"<script text>\" [-v]\n");
		return 1;
	}

	ScriptParser parser;
	bool inlineScript = (strcmp(argv[1], "-e") == 0);

	if (inlineScript && argc < 3)
	{
		fprintf(stderr, "usage: script -e \"<script text>\" [-v]\n");
		return 1;
	}

	const char *arg = inlineScript ? argv[2] : argv[1];
	int vArgIndex = inlineScript ? 3 : 2;

	parser.yydebug = (argc > vArgIndex && strcmp(argv[vArgIndex], "-v") == 0);

	if (inlineScript)
	{
		// parseData() stores the buffer pointer as-is (no internal copy), and
		// runs the whole parse synchronously before returning, so it's safe
		// for 'buf' to simply live for the duration of this call
		std::vector<char> buf(arg, arg + strlen(arg) + 1);
		parser.parseData(buf.data(), "<inline>", nullptr);
	}
	else
	{
		parser.parseFile(arg);
	}

	if (parser.getErrorCount() > 0)
	{
		fprintf(stderr, "%u error(s) found.\n", parser.getErrorCount());
		return 1;
	}

	return 0;
}
