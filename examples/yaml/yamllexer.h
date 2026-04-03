#pragma once

#include <vector>
#include <queue>
#include <string>
#include "../../lexer.h"

// -------------------------------------------------------------------------
// Additional token values for YAML (extend TV_USER from lexer.h)
// -------------------------------------------------------------------------
enum
{
    TV_KEY       = TV_USER,  // bare word immediately followed by ':'  (block only)
    TV_SCALAR,               // bare word NOT followed by ':'
    TV_YAML_INT,             // integer literal
    TV_YAML_FLOAT,           // floating-point literal
    TV_YAML_TRUE,            // true / yes / on  (case-insensitive)
    TV_YAML_FALSE,           // false / no / off (case-insensitive)
    TV_YAML_NULL,            // null / ~
    TV_INDENT,               // indentation depth increased
    TV_DEDENT,               // indentation depth decreased
    TV_NEWLINE,              // logical end of content line
    TV_DASH,                 // "- " introducing a block sequence item
};

// -------------------------------------------------------------------------
// YAMLLexer
//
// Overrides yylex() completely to implement:
//   • INDENT / DEDENT tokens derived from leading-space changes
//   • KEY  token for bare-word mapping keys  (word immediately before ':')
//   • DASH token for block sequence items    ("- " at block level)
//   • case-insensitive keyword recognition   (true/false/null/…)
//
// In flow context (inside { } or [ ]) indentation is ignored and newlines
// are treated as whitespace, exactly as in the YAML specification.
// -------------------------------------------------------------------------
class YAMLLexer : public LexicalAnalyzer
{
    std::vector<int> m_indentStack;  // stack of indent levels; starts at {0}
    std::queue<int>  m_pending;      // buffered tokens (INDENT/DEDENT/EOF)
    int              m_flowDepth;    // nesting of { } and [ ]
    bool             m_atBOL;        // true when positioned at beginning of line

    // Process beginning-of-line: count spaces, queue INDENT/DEDENT tokens.
    void processBOL();

    // Skip from the current position to the end of the current line.
    void skipToEOL();

    // Helpers for reading specific token kinds.
    int readString();
    int readNumber(int firstChar);
    int readWord(int firstChar);

public:
    YAMLLexer(TokenTable *tt, BaseParser *p, YYSTYPE *v);

    int yylex() override;
};
