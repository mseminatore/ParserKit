%{
//
// yaml.y  —  YAML grammar for the bnf parser generator
//
// Covers a useful subset of YAML 1.2:
//   • Block mappings and sequences (indentation-based)
//   • Flow mappings { k: v, ... } and flow sequences [ v, ... ]
//   • Scalar types: double-quoted strings, bare-word scalars, integers,
//     floats, booleans (true/false/yes/no/on/off), and null (~)
//
// Build:
//   bnf -o yamlparser yaml.y
//   c++ -std=c++11 yamlparser.cpp tableparser.cpp -o yaml
// Run:
//   ./yaml test.yaml
//   ./yaml test.yaml -v      (verbose: shows parse-table decisions)
//
// LL(1) grammar notes
// -------------------
// YAML block mappings are LL(2) if bare-word keys share the SCALAR token
// with plain values.  The lexer resolves this by emitting KEY (not SCALAR)
// when a bare word is immediately followed by ':' in block context,
// removing the ambiguity from the grammar.
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <queue>
#include <cstdlib>
#include <cstring>

struct YYSTYPE {
    std::string text;
    int         ival = 0;
    float       fval = 0.0f;

    void empty()                           { text = ""; ival = 0; fval = 0.0f; }
    void setText(const std::string &s)     { text = s; }
    void setInt(int i)                     { ival = i; }
    void setNumber(float f)                { fval = f; }

    const std::string &asString() const    { return text; }
    int                asInt()    const    { return ival; }
    float              asNumber() const    { return fval; }
} yylval;

%}

// -------------------------------------------------------------------------
// Token declarations
//
//  KEY       — bare word immediately followed by ':' (block context only)
//  SCALAR    — bare word that is NOT followed by ':'
//  STRING    — double-quoted string literal
//  INT       — integer literal
//  FLOAT     — floating-point literal
//  YAML_TRUE — true / yes / on  (case-insensitive)
//  YAML_FALSE— false / no / off (case-insensitive)
//  YAML_NULL — null / ~
//  INDENT    — indentation increases to a new level
//  DEDENT    — indentation returns to the previous level
//  NEWLINE   — logical end of a content line
//  DASH      — "- " introducing a block sequence item
// -------------------------------------------------------------------------
%token KEY SCALAR STRING INT FLOAT YAML_TRUE YAML_FALSE YAML_NULL
%token INDENT DEDENT NEWLINE DASH

%start document

%%

// -------------------------------------------------------------------------
// Top-level document: either a block structure or a single inline value
// -------------------------------------------------------------------------
document: block_content
        | scalar_value NEWLINE
        ;

// -------------------------------------------------------------------------
// Block-level structures
// -------------------------------------------------------------------------
block_content: block_mapping
             | block_sequence
             ;

block_mapping: block_pair more_block_pairs
             ;

block_pair: KEY block_value
          ;

// A block value is either an inline scalar on the same line (followed by
// NEWLINE), or a nested block structure on the next line(s).
block_value: scalar_value NEWLINE
           | NEWLINE INDENT block_content DEDENT
           ;

more_block_pairs:
               | block_pair more_block_pairs
               ;

block_sequence: block_item more_block_items
              ;

block_item: DASH block_seq_value
          ;

block_seq_value: scalar_value NEWLINE
               | NEWLINE INDENT block_content DEDENT
               ;

more_block_items:
               | block_item more_block_items
               ;

// -------------------------------------------------------------------------
// Scalar values — used in both block and flow contexts
// -------------------------------------------------------------------------
scalar_value: STRING
                { printf("string: \"%s\"\n", $1.asString().c_str()); }
            | SCALAR
                { printf("scalar: %s\n", $1.asString().c_str()); }
            | INT
                { printf("int: %d\n", $1.asInt()); }
            | FLOAT
                { printf("float: %f\n", $1.asNumber()); }
            | YAML_TRUE
                { printf("bool: true\n"); }
            | YAML_FALSE
                { printf("bool: false\n"); }
            | YAML_NULL
                { printf("null\n"); }
            | flow_mapping
            | flow_sequence
            ;

// -------------------------------------------------------------------------
// Flow-style structures  { k: v, ... }  and  [ v, ... ]
// -------------------------------------------------------------------------
flow_mapping: '{' flow_pairs '}'
            ;

flow_pairs:
          | flow_pair more_flow_pairs
          ;

flow_pair: flow_key ':' scalar_value
         ;

// In flow context the lexer emits SCALAR (not KEY) for bare words,
// so both quoted strings and bare words are valid as mapping keys.
flow_key: STRING
        | SCALAR
        ;

more_flow_pairs:
              | ',' flow_pair more_flow_pairs
              ;

flow_sequence: '[' flow_values ']'
             ;

flow_values:
           | scalar_value more_flow_values
           ;

more_flow_values:
               | ',' scalar_value more_flow_values
               ;

%%

// =========================================================================
// YAML lexer
//
// Manages an indentation stack and emits INDENT / DEDENT tokens when the
// leading-space count changes between lines.  In flow context (inside { }
// or [ ]) indentation is ignored and newlines are treated as whitespace.
// =========================================================================

static FILE   *YYIN       = stdin;
static unsigned yylineno  = 1;

// Indentation tracking
static std::vector<int> indent_stack = {0};
static std::queue<int>  pending;          // buffered INDENT/DEDENT/etc.
static int              flow_depth  = 0;  // nesting depth of { } and [ ]
static bool             at_bol      = true; // at beginning of line

static int  xgetc()        { return fgetc(YYIN); }
static void xungetc(int c) { ungetc(c, YYIN); }

// Skip from current position to end of line (does not consume the '\n').
static void skip_to_eol()
{
    int c;
    while ((c = xgetc()) != '\n' && c != EOF)
        ;
    xungetc(c);
}

// Count leading spaces on the next non-empty, non-comment line.
// Queues INDENT / DEDENT tokens as appropriate.  Leaves the stream
// positioned at the first non-space character of the real line.
static void process_bol()
{
    for (;;)
    {
        int indent = 0;
        int c;

        while ((c = xgetc()) == ' ')
            ++indent;

        if (c == EOF)
        {
            while (indent_stack.size() > 1)
            {
                indent_stack.pop_back();
                pending.push(TS_DEDENT);
            }
            pending.push(0); // EOF sentinel
            at_bol = false;
            return;
        }

        if (c == '\r') { int n = xgetc(); if (n != '\n') xungetc(n); c = '\n'; }

        if (c == '\n') { ++yylineno; continue; }   // blank line — skip
        if (c == '#')  { skip_to_eol(); continue; } // comment line — skip

        // Found a real content line
        xungetc(c);

        if (indent > indent_stack.back())
        {
            indent_stack.push_back(indent);
            pending.push(TS_INDENT);
        }
        else
        {
            while (indent < indent_stack.back())
            {
                indent_stack.pop_back();
                pending.push(TS_DEDENT);
            }
        }

        at_bol = false;
        return;
    }
}

// Read a double-quoted string into yylval.text.
static int read_string()
{
    std::string s;
    int c;
    while ((c = xgetc()) != '"' && c != EOF)
    {
        if (c == '\\')
        {
            c = xgetc();
            switch (c)
            {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            }
        }
        s += (char)c;
    }
    yylval.setText(s);
    return TS_STRING;
}

// Read an integer or float starting with 'first' (already consumed).
static int read_number(int first)
{
    std::string s;
    s += (char)first;
    int c;
    bool is_float = false;
    while (c = xgetc(), isdigit(c) || c == '.')
    {
        if (c == '.') is_float = true;
        s += (char)c;
    }
    xungetc(c);

    if (is_float)
    {
        yylval.setNumber((float)atof(s.c_str()));
        return TS_FLOAT;
    }
    yylval.setInt(atoi(s.c_str()));
    return TS_INT;
}

// Read a bare word starting with 'first' (already consumed).
// Returns KEY, a keyword token, or SCALAR.
static int read_word(int first)
{
    std::string s;
    s += (char)first;
    int c;
    while (c = xgetc(), isalnum(c) || c == '_' || c == '-')
        s += (char)c;
    // 'c' is the first character after the word — NOT yet unget'd

    // In block context, check for mapping key: "word:"
    if (flow_depth == 0 && c == ':')
    {
        int next = xgetc();
        bool is_key = (next == ' ' || next == '\t' ||
                       next == '\n' || next == '\r' || next == EOF);
        xungetc(next);
        if (is_key)
        {
            // ':' is consumed as part of the KEY token
            yylval.setText(s);
            return TS_KEY;
        }
    }

    xungetc(c);

    // Keyword check (case-insensitive)
    std::string lc = s;
    for (auto &ch : lc) ch = (char)tolower((unsigned char)ch);

    if (lc == "true"  || lc == "yes" || lc == "on")  { yylval.empty(); return TS_YAML_TRUE;  }
    if (lc == "false" || lc == "no"  || lc == "off") { yylval.empty(); return TS_YAML_FALSE; }
    if (lc == "null")                                 { yylval.empty(); return TS_YAML_NULL;  }

    yylval.setText(s);
    return TS_SCALAR;
}

// Main lexer entry point.
int yylex()
{
    // Drain any buffered tokens first (INDENT / DEDENT / EOF)
    if (!pending.empty())
    {
        int t = pending.front();
        pending.pop();
        return t;
    }

    // Process start-of-line indentation in block context
    if (at_bol && flow_depth == 0)
    {
        process_bol();
        if (!pending.empty())
        {
            int t = pending.front();
            pending.pop();
            return t;
        }
    }

    int c;

    // Skip inline whitespace (spaces and tabs, but not newlines)
    while ((c = xgetc()) == ' ' || c == '\t')
        ;

    if (c == EOF) return 0;

    // Newline handling
    if (c == '\n' || c == '\r')
    {
        if (c == '\r') { int n = xgetc(); if (n != '\n') xungetc(n); }
        ++yylineno;

        if (flow_depth > 0) return yylex(); // newlines are whitespace in flow

        at_bol = true;
        return TS_NEWLINE;
    }

    // Line comment
    if (c == '#') { skip_to_eol(); return yylex(); }

    // Flow context delimiters — track nesting depth
    if (c == '{') { ++flow_depth; return c; }
    if (c == '}') { if (flow_depth > 0) --flow_depth; return c; }
    if (c == '[') { ++flow_depth; return c; }
    if (c == ']') { if (flow_depth > 0) --flow_depth; return c; }

    // Simple punctuation used in flow context
    if (c == ',' || c == ':') return c;

    // Quoted string
    if (c == '"') return read_string();

    // YAML null shorthand
    if (c == '~') { yylval.empty(); return TS_YAML_NULL; }

    // Block sequence dash: "- " or "-\n" at block level
    if (c == '-' && flow_depth == 0)
    {
        int next = xgetc();
        if (next == ' ' || next == '\t' || next == '\n' || next == '\r')
        {
            xungetc(next);
            return TS_DASH;
        }
        if (isdigit(next)) return read_number('-');
        xungetc(next);
        return c; // lone '-', pass through
    }

    // Numeric literals
    if (isdigit(c)) return read_number(c);

    // Bare words (keys, scalars, keywords)
    if (isalpha(c) || c == '_') return read_word(c);

    // Any other character passed through as-is
    return c;
}

void yyerror(const char *str)
{
    fprintf(stderr, "yaml: error (line %u): %s\n", yylineno, str);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: yaml <file.yaml> [-v]\n");
        return 1;
    }

    YYIN = fopen(argv[1], "r");
    if (!YYIN)
    {
        fprintf(stderr, "yaml: cannot open '%s'\n", argv[1]);
        return 1;
    }

    bool verbose = (argc > 2 && strcmp(argv[2], "-v") == 0);

    yamlparser parser(yylex);
    parser.setDebug(verbose);
    parser.yyparse();

    printf("\nParsed '%s' successfully.\n", argv[1]);

    fclose(YYIN);
    return 0;
}
