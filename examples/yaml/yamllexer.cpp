#define _CRT_SECURE_NO_WARNINGS

#include <cctype>
#include <cstdlib>
#include "yamllexer.h"
#include "../../baseparser.h"

// -------------------------------------------------------------------------
// Construction
// -------------------------------------------------------------------------
YAMLLexer::YAMLLexer(TokenTable *tt, BaseParser *p, YYSTYPE *v)
    : LexicalAnalyzer(tt, p, v)
    , m_flowDepth(0)
    , m_atBOL(true)
{
    m_indentStack.push_back(0);
}

// -------------------------------------------------------------------------
// Skip to end of line (leaves '\n' in the stream to be consumed later)
// -------------------------------------------------------------------------
void YAMLLexer::skipToEOL()
{
    int c;
    while ((c = getChar()) != '\n' && c != EOF)
        ;
    ungetChar(c);
}

// -------------------------------------------------------------------------
// processBOL — called when m_atBOL is true (block context only).
// Counts the leading spaces on the next non-blank, non-comment line,
// then queues INDENT / DEDENT tokens as necessary.
// -------------------------------------------------------------------------
void YAMLLexer::processBOL()
{
    for (;;)
    {
        int indent = 0;
        int c;

        while ((c = getChar()) == ' ')
            ++indent;

        if (c == EOF)
        {
            // Flush any open indentation levels
            while (m_indentStack.size() > 1)
            {
                m_indentStack.pop_back();
                m_pending.push(TV_DEDENT);
            }
            m_pending.push(0); // EOF sentinel
            m_atBOL = false;
            return;
        }

        if (c == '\r') { int n = getChar(); if (n != '\n') ungetChar(n); c = '\n'; }

        if (c == '\n')
        {
            // Blank line — skip
            m_fdStack.back().yylineno++;
            continue;
        }

        if (c == '#')
        {
            // Comment line — skip
            skipToEOL();
            continue;
        }

        // Found a real content line
        ungetChar(c);

        if (indent > m_indentStack.back())
        {
            m_indentStack.push_back(indent);
            m_pending.push(TV_INDENT);
        }
        else
        {
            while (indent < m_indentStack.back())
            {
                m_indentStack.pop_back();
                m_pending.push(TV_DEDENT);
            }
        }

        m_atBOL = false;
        return;
    }
}

// -------------------------------------------------------------------------
// readString — reads a double-quoted string; returns TV_STRING
// -------------------------------------------------------------------------
int YAMLLexer::readString()
{
    std::string s;
    int c;
    while ((c = getChar()) != '"' && c != EOF)
    {
        if (c == '\\')
        {
            c = getChar();
            switch (c)
            {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            case '"': c = '"';  break;
            }
        }
        s += (char)c;
    }
    // Install the string in the symbol table so m_yylval->sym is set
    m_yylval->sym = m_pParser->installSymbol(const_cast<char*>(s.c_str()), stStringLiteral);
    return TV_STRING;
}

// -------------------------------------------------------------------------
// readNumber — reads an integer or float starting with 'firstChar'
// -------------------------------------------------------------------------
int YAMLLexer::readNumber(int firstChar)
{
    std::string s;
    s += (char)firstChar;
    int c;
    bool is_float = false;
    while (c = getChar(), isdigit(c) || c == '.')
    {
        if (c == '.') is_float = true;
        s += (char)c;
    }
    ungetChar(c);

    if (is_float)
    {
        m_yylval->fval = (float)atof(s.c_str());
        return TV_YAML_FLOAT;
    }
    m_yylval->ival = atoi(s.c_str());
    return TV_YAML_INT;
}

// -------------------------------------------------------------------------
// readWord — reads a bare word and classifies it as:
//   TV_KEY         if in block context and immediately followed by ':'
//   TV_YAML_TRUE/FALSE/NULL  if a recognised keyword
//   TV_SCALAR      otherwise
// -------------------------------------------------------------------------
int YAMLLexer::readWord(int firstChar)
{
    std::string s;
    s += (char)firstChar;
    int c;
    while (c = getChar(), isalnum(c) || c == '_' || c == '-')
        s += (char)c;
    // 'c' is the first character after the word, not yet unget'd

    // Check for mapping key in block context: word immediately followed by ':'
    if (m_flowDepth == 0 && c == ':')
    {
        int next = getChar();
        bool is_key = (next == ' ' || next == '\t' ||
                       next == '\n' || next == '\r' || next == EOF);
        ungetChar(next);

        if (is_key)
        {
            // Consume the ':' as part of the KEY token
            m_yylval->sym = m_pParser->installSymbol(
                const_cast<char*>(s.c_str()), stUndef);
            return TV_KEY;
        }
    }

    ungetChar(c);

    // Case-insensitive keyword recognition
    std::string lc = s;
    for (auto &ch : lc) ch = (char)tolower((unsigned char)ch);

    if (lc == "true"  || lc == "yes" || lc == "on")  return TV_YAML_TRUE;
    if (lc == "false" || lc == "no"  || lc == "off") return TV_YAML_FALSE;
    if (lc == "null")                                  return TV_YAML_NULL;

    m_yylval->sym = m_pParser->installSymbol(
        const_cast<char*>(s.c_str()), stUndef);
    return TV_SCALAR;
}

// -------------------------------------------------------------------------
// yylex — main entry point
// -------------------------------------------------------------------------
int YAMLLexer::yylex()
{
    // Drain any buffered tokens (INDENT / DEDENT / EOF)
    if (!m_pending.empty())
    {
        int t = m_pending.front();
        m_pending.pop();
        return t;
    }

    // Process start-of-line indentation (block context only)
    if (m_atBOL && m_flowDepth == 0)
    {
        processBOL();
        if (!m_pending.empty())
        {
            int t = m_pending.front();
            m_pending.pop();
            return t;
        }
    }

    int c;

    // Skip inline whitespace
    while ((c = getChar()) == ' ' || c == '\t')
        ;

    if (c == EOF) return TV_DONE;

    // Newline
    if (c == '\n' || c == '\r')
    {
        if (c == '\r') { int n = getChar(); if (n != '\n') ungetChar(n); }
        m_fdStack.back().yylineno++;

        if (m_flowDepth > 0) return yylex(); // newlines are whitespace in flow

        m_atBOL = true;
        return TV_NEWLINE;
    }

    // Inline comment
    if (c == '#') { skipToEOL(); return yylex(); }

    // Flow context delimiters
    if (c == '{') { ++m_flowDepth; return c; }
    if (c == '}') { if (m_flowDepth > 0) --m_flowDepth; return c; }
    if (c == '[') { ++m_flowDepth; return c; }
    if (c == ']') { if (m_flowDepth > 0) --m_flowDepth; return c; }

    // Flow punctuation
    if (c == ',' || c == ':') return c;

    // Quoted string
    if (c == '"') return readString();

    // YAML null shorthand
    if (c == '~') return TV_YAML_NULL;

    // Block sequence dash: "- " or "-\n" at block level
    if (c == '-' && m_flowDepth == 0)
    {
        int next = getChar();
        if (next == ' ' || next == '\t' || next == '\n' || next == '\r')
        {
            ungetChar(next);
            return TV_DASH;
        }
        if (isdigit(next)) return readNumber('-');
        ungetChar(next);
        return c;
    }

    // Numeric literal
    if (isdigit(c)) return readNumber(c);

    // Bare word
    if (isalpha(c) || c == '_') return readWord(c);

    return c; // pass any other character through unchanged
}
