// yaml.cpp — driver for the standalone YAML parser example
//
// Usage:
//   yaml <file.yaml>        Parse and dump the YAML value tree
//   yaml <file.yaml> -v     Verbose: also trace parser rule calls

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include "yamlparser.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: yaml <file.yaml> [-v]\n");
        return 1;
    }

    YAMLParser parser;

    if (argc > 2 && strcmp(argv[2], "-v") == 0)
        parser.yydebug = true;

    parser.parseFile(argv[1]);

    if (parser.getErrorCount() > 0)
    {
        fprintf(stderr, "%u error(s) found.\n", parser.getErrorCount());
        return 1;
    }

    printf("Parsed '%s' successfully.\n", argv[1]);
    return 0;
}
