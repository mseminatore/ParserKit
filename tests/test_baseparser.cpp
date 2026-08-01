#include <cstring>
#include "../baseparser.h"
#include "testy/test.h"

namespace {

enum { TV_TRUE = TV_USER, TV_FALSE };

TokenTable g_tokenTable[] = {
    { "true",  TV_TRUE  },
    { "false", TV_FALSE },
    { nullptr, TV_DONE  }
};

// LexicalAnalyzer::yyerror() calls exit() by default (see lexer.cpp), which
// would kill the test process on a deliberate mismatch. Override it so error
// paths can be exercised without terminating the test binary.
class NonFatalLexer : public LexicalAnalyzer
{
public:
    using LexicalAnalyzer::LexicalAnalyzer;
    void yyerror(const char *s) override { puts(s); }
};

// Minimal parser subclass, following the pattern documented in CLAUDE.md:
// yyparse() must call BaseParser::yyparse() first to prime the lookahead.
class TestParser : public BaseParser
{
public:
    TestParser() : BaseParser(std::unique_ptr<SymbolTable>(new SymbolTable()))
    {
        m_lexer.reset(new NonFatalLexer(g_tokenTable, this, &yylval));
    }

    int yyparse() override
    {
        BaseParser::yyparse();
        match(TV_TRUE);
        return 0;
    }
};

char *dup(const char *text)
{
    static char buf[256];
    strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    return buf;
}

} // namespace

//------------------------------------------------------
void test_baseparser()
{
    MODULE("BaseParser");

    SUITE("match success");
    {
        TestParser parser;
        parser.parseData(dup("true"), "test", nullptr);
        TEST(parser.getErrorCount() == 0);
    }

    SUITE("match failure");
    {
        TestParser parser;
        parser.parseData(dup("false"), "test", nullptr);
        TEST(parser.getErrorCount() == 1);
    }

    SUITE("symbol delegation");
    {
        TestParser parser;
        SymbolEntry *pInstalled = parser.installSymbol((char *)"foo", stInteger);
        TEST(pInstalled != nullptr);
        TEST(parser.lookupSymbol((char *)"foo") == pInstalled);
    }
}
