// bnf.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "bnfparser.h"

//
// Command line switches
//
static bool g_bDebug = false;
static FILE *yyout = stdout;
static FILE *yyhout = stdout;
static std::string outputFile = "ytab";

//
// show usage
//
void usage()
{
	printf("usage: bnf [options] filename\n");
	exit(0);
}

//
// get options from the command line
//
int getopt(int n, char *args[])
{
	int i;
	for (i = 1; args[i][0] == '-'; i++)
	{
		if (args[i][1] == 'v')
			g_bDebug = true;

		if (args[i][1] == 'o')
		{
			outputFile = args[++i];
			std::string file = outputFile + ".cpp";
			yyout = fopen(file.c_str(), "wt");

			file = outputFile + ".h";
			yyhout = fopen(file.c_str(), "wt");
		}
	}

	return i;
}

//
//
//
int main(int argc, char* argv[])
{
	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	BNFParser parser;

	parser.yydebug = g_bDebug;
	parser.yyout = yyout;
	parser.yyhout = yyhout;
	parser.setFileName(outputFile);

	parser.parseFile(argv[iFirstArg]);

	return 0;
}