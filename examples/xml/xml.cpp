// xml.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "xmlparser.h"

//
// Command line switches
//
bool g_bDebug = false;

//
// show usage
//
void usage()
{
	printf("usage: xml [options] filename\n");
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

	XMLParser parser;

	parser.yydebug = g_bDebug;

	parser.parseFile(argv[iFirstArg]);

	return 0;
}