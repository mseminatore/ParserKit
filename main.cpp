//
//
//

#include <stdio.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "cparser.h"

//
// Compiler switches
//
bool g_bOptimize = false;
bool g_bDumpPhaseIR = false;

//
// show usage
//
void usage()
{
	printf("usage: main [options] filename\n");
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
		if (args[i][1] == 'O')
			g_bOptimize = true;

		if (args[i][1] == 'd')
			g_bDumpPhaseIR = true;
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

	auto parser = std::make_unique<CParser>();

	parser->Init();

	// TODO - loop over files on the command line
	parser->Parse(argv[iFirstArg]);

	return 0;
}

