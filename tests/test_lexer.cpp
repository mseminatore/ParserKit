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

// LexicalAnalyzer's constructor requires a BaseParser*, but a plain
// BaseParser (with no lexer of its own) is sufficient here since these
// tests only exercise well-formed input and never hit an error path
// that would call back through the parser's own (null) lexer.
struct LexerFixture
{
    SymbolTable symbolTable;
    BaseParser parser;
    YYSTYPE yylval;
    LexicalAnalyzer lexer;

    LexerFixture()
        : parser(std::unique_ptr<SymbolTable>(new SymbolTable()))
        , lexer(g_tokenTable, &parser, &yylval)
    {
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
void test_lexer()
{
    MODULE("LexicalAnalyzer");

    SUITE("basic tokens");
    {
        LexerFixture fixture;
        fixture.lexer.setData(dup("true 123 3.14 \"hello\" ident"), "test", nullptr);

        TEST(fixture.lexer.yylex() == TV_TRUE);

        TEST(fixture.lexer.yylex() == TV_INTVAL);
        TEST(fixture.yylval.ival == 123);

        TEST(fixture.lexer.yylex() == TV_FLOATVAL);
        TEST(EQUAL_EPSILON(fixture.yylval.fval, 3.14f));

        TEST(fixture.lexer.yylex() == TV_STRING);
        TEST(fixture.yylval.sym->lexeme == "hello");

        TEST(fixture.lexer.yylex() == TV_ID);
        TEST(fixture.yylval.sym->lexeme == "ident");

        TEST(fixture.lexer.yylex() == TV_DONE);
    }

    SUITE("hex numbers");
    {
        LexerFixture fixture;
        fixture.lexer.setHexNumbers(true);
        fixture.lexer.setData(dup("0x1F"), "test", nullptr);

        TEST(fixture.lexer.yylex() == TV_INTVAL);
        TEST(fixture.yylval.ival == 31);
    }

    SUITE("char literals");
    {
        LexerFixture fixture;
        fixture.lexer.setCharLiterals(true);
        fixture.lexer.setData(dup("'A'"), "test", nullptr);

        TEST(fixture.lexer.yylex() == TV_CHARVAL);
        TEST(fixture.yylval.char_val == 'A');
    }

    SUITE("C-style comments");
    {
        LexerFixture fixture;
        fixture.lexer.setCStyleComments(true);
        fixture.lexer.setData(dup("/* a comment */ 42"), "test", nullptr);

        TEST(fixture.lexer.yylex() == TV_INTVAL);
        TEST(fixture.yylval.ival == 42);
    }
}
