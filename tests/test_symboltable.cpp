#include <string>
#include <map>
#include <list>
#include "../symboltable.h"
#include "testy/test.h"

//------------------------------------------------------
void test_symboltable()
{
    MODULE("SymbolTable");

    SUITE("install/lookup");
    {
        SymbolTable table;

        SymbolEntry *pInstalled = table.install("foo", stInteger);
        TEST(pInstalled != nullptr);
        TEST(pInstalled->lexeme == "foo");
        TEST(pInstalled->type == stInteger);

        SymbolEntry *pFound = table.lookup("foo");
        TEST(pFound == pInstalled);

        TEST(table.lookup("does_not_exist") == nullptr);
    }

    SUITE("scoping");
    {
        SymbolTable table;

        table.install("outer", stInteger);

        table.push();
        SymbolEntry *pInner = table.install("inner", stFloat);
        TEST(table.lookup("inner") == pInner);
        TEST(table.lookup("outer") != nullptr);
        table.pop();

        TEST(table.lookup("inner") == nullptr);
        TEST(table.lookup("outer") != nullptr);
    }

    SUITE("shadowing");
    {
        SymbolTable table;

        SymbolEntry *pOuter = table.install("x", stInteger);

        table.push();
        SymbolEntry *pInner = table.install("x", stFloat);

        SymbolEntry *pFound = table.lookup("x");
        TEST(pFound == pInner);
        TEST(pFound->type == stFloat);

        table.pop();

        pFound = table.lookup("x");
        TEST(pFound == pOuter);
        TEST(pFound->type == stInteger);
    }

    SUITE("reverse_lookup");
    {
        SymbolTable table;

        SymbolEntry *pInstalled = table.install("answer", stInteger);
        pInstalled->ival = 42;

        SymbolEntry *pFound = table.reverse_lookup(42);
        TEST(pFound == pInstalled);
    }

    SUITE("dumpUnreferencedSymbolsAtCurrentLevel");
    {
        SymbolTable table;

        SymbolEntry *pInstalled = table.install("unused", stInteger);
        TEST(table.dumpUnreferencedSymbolsAtCurrentLevel() == 1);

        pInstalled->isReferenced = 1;
        TEST(table.dumpUnreferencedSymbolsAtCurrentLevel() == 0);
    }
}
