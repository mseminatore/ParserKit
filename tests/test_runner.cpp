#include "testy/test.h"

void test_symboltable();
void test_lexer();
void test_baseparser();

void test_main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    test_symboltable();
    test_lexer();
    test_baseparser();
}
