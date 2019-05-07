//
//
//

#include <stdio.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "jsonparser.h"

//
// Command line switches
//
bool g_bDebug = false;

//
// show usage
//
void usage()
{
	printf("usage: json [options] filename\n");
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

	JSONParser parser;
	
	parser.yydebug = g_bDebug;

	parser.Parse(argv[iFirstArg]);

	return 0;
}

