# ParserKit

[![CI](https://github.com/mseminatore/ParserKit/actions/workflows/ci.yml/badge.svg)](https://github.com/mseminatore/ParserKit/actions/workflows/ci.yml)

A simple, modern C++ library for creating top-down recursive-descent predictive
parsers for LL(1) grammars.

## What is ParserKit?

It is perhaps best to start with what this library is not. It is not intended to
be, or to replace, a commercial product. Nor is it exhaustively tested.

It is, however, intended to offer up a collection of utility routines and classes 
to make creating your own parsers a more practical exercise. It includes classes
 for:

* Customizable Lexical Analysis
* A multi-level symbol table
* An extensible parsing framework

## Why create this library?

I have always been fascinated by the concepts behind compilers and 
interpreters. Over time, I gathered knowledge where and when I could and over time taught
myself the fundamentals of parsing technology. I've used what I've 
learned to create several data serialization formats, small scripting languages,
assemblers, and compilers.

Because parsing concepts can be difficult to learn and explain, and the technology
still seems mystical to many. I decided to share this library in the hope
that it may be helpful to others.

## How can I learn more about parsing?

I've found that there is no single textbook that clearly presents all that you 
want and need to know about parsing. For those just starting out I would recommend:

1. The [Dragon Book](https://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811/ref=sr_1_1?crid=2M6JW4SE1LFYA&dchild=1&keywords=aho+sethi+ullman&qid=1592861659&sprefix=aho+set%2Caps%2C215&sr=8-1) is one of the classical texts on the subject.
2. Good coverage of the GNU versions of Yacc and Lex is [Flex and Bison](https://www.amazon.com/flex-bison-Text-Processing-Tools/dp/0596155972/ref=sr_1_1?dchild=1&keywords=flex+and+bison&qid=1592863889&sr=8-1)
3. Another good source of information on Yacc and Lex is [The Unix Programming Environment](https://www.amazon.com/Unix-Programming-Environment-Prentice-Hall-Software/dp/013937681X/ref=sr_1_2?crid=2Y5FH064KCR8X&dchild=1&keywords=the+unix+programming+environment&qid=1592861830&sprefix=the+unix+program%2Caps%2C212&sr=8-2)
4. [Modern Compiler Implementation in C](https://www.amazon.com/Modern-Compiler-Implementation-Andrew-Appel/dp/817596071X/ref=sxts_sxwds-bia-wc-p13n1_0?crid=2CGBMJ614Z1BF&cv_ct_cx=modern+compiler+implementation+in+c&dchild=1&keywords=modern+compiler+implementation+in+c&pd_rd_i=817596071X&pd_rd_r=4c3593d8-34db-4796-929b-b84a1ac8cd26&pd_rd_w=Yh0K9&pd_rd_wg=3RWhe&pf_rd_p=1da5beeb-8f71-435c-b5c5-3279a6171294&pf_rd_r=4AF9BXMZ36CGJMNM3FR4&psc=1&qid=1592861915&sprefix=modern+compiler%2Caps%2C214&sr=1-1-70f7c15d-07d8-466a-b325-4be35d7258cc) provides a great overview of different parsing methods as well as 
other aspects of compiler implementation like register allocation and code generation.

# Library Documentation

The documentation for the library is a work in progress.

## Build

```bash
# Makefile (library only)
make

# Makefile (library + all examples)
make all

# CMake
cmake -B build && cmake --build build
```

---

## Token Types

Predefined token values returned by `yylex()`:

| Constant | Value | Description |
|----------|-------|-------------|
| `TV_ERROR` | 256 | Lexer error |
| `TV_DONE` | 257 | End of input |
| `TV_INTVAL` | 258 | Integer literal — value in `yylval.ival` |
| `TV_FLOATVAL` | 259 | Float literal — value in `yylval.fval` |
| `TV_CHARVAL` | 260 | Char literal — value in `yylval.char_val` |
| `TV_STRING` | 261 | String literal — symbol in `yylval.sym` |
| `TV_ID` | 262 | Identifier — symbol in `yylval.sym` |
| `TV_USER` | 263 | First user-defined token value |

Single-character tokens use their ASCII value directly (e.g., `'{'`, `':'`).

User-defined tokens should start at `TV_USER`:

```cpp
enum { TV_TRUE = TV_USER, TV_FALSE, TV_NULL };
```

---

## Key Types

### `TokenTable`
Maps lexeme strings to token values. The array must end with a `{ nullptr, TV_DONE }` sentinel:

```cpp
TokenTable myTokens[] = {
    { "true",  TV_TRUE  },
    { "false", TV_FALSE },
    { nullptr, TV_DONE  }  // sentinel
};
```

### `YYSTYPE`
Semantic value union filled by the lexer for each token:

```cpp
union YYSTYPE {
    int         ival;      // TV_INTVAL
    float       fval;      // TV_FLOATVAL
    char        char_val;  // TV_CHARVAL
    SymbolEntry *sym;      // TV_STRING, TV_ID  (lexeme in sym->lexeme)
    TokenTable  *ptt;      // keyword entry
};
```

### `SymbolEntry`
An entry in the symbol table:

| Field | Type | Description |
|-------|------|-------------|
| `lexeme` | `std::string` | Text of the symbol |
| `type` | `SymbolType` | Symbol type (starts at `stUndef`; user types start at `stUser`) |
| `srcLine` | `int` | Source line number where first seen |
| `srcFile` | `std::string` | Source file name |
| `ival` / `fval` / `char_val` / `bval` | union | Literal value (if applicable) |
| `isReferenced` | `unsigned:1` | Set to `1` when the symbol is referenced |
| `global` | `bool` | Whether this is a global symbol |

### `Position`
Captures a source location for error reporting:

```cpp
Position pos(srcFile, srcLine, srcColumn);
parser.yyerror(pos, "unexpected token '%s'", tok);
```

---

## Classes

### `LexicalAnalyzer`

Tokenizes input from a file or an in-memory buffer. Subclass it to add custom token handling.

#### Constructor

```cpp
LexicalAnalyzer(TokenTable *tokenTable, BaseParser *parser, YYSTYPE *yylval);
```

Registers the token table, owning parser, and semantic value destination.

#### Input

| Method | Description |
|--------|-------------|
| `int pushFile(const char *path)` | Open a file and push it onto the input stack. Returns 0 on success, -1 on error. |
| `int popFile()` | Close the current input and pop to the previous one. Returns `EOF` when the stack is empty. |
| `int setData(char *data, const char *fileName, void *userData)` | Parse from a `char*` buffer instead of a file. `userData` is passed to `freeData()` when done. |
| `virtual void freeData(void *userData)` | Override to free `userData` when an in-memory input is popped. Default asserts if non-null. |

#### Lexer

| Method | Description |
|--------|-------------|
| `virtual int yylex()` | Return the next token. Override to extend or replace lexing behaviour. |
| `virtual int specialTokens(int chr)` | Called for characters not handled by the default rules. Override to add multi-character punctuation (e.g., `%%`, `->`, `::=`). Default returns single-char tokens or `TV_DONE` at EOF. |
| `virtual bool isidval(int c)` | Returns `true` if `c` is valid inside an identifier. Default: alphanumeric or `_`. |
| `virtual bool iswhitespace(int c)` | Returns `true` if `c` is whitespace. Default: space, tab, `\n`, `\r`. |

#### Configuration

Feature flags set in the constructor of a subclass:

| Flag | Default | Description |
|------|---------|-------------|
| `m_bCPPComments` | `false` | Enable `// …` line comments |
| `m_bCStyleComments` | `false` | Enable `/* … */` block comments |
| `m_bUnixComments` | `false` | Enable `#` line comments |
| `m_bASMComments` | `false` | Enable `;` line comments |
| `m_bHexNumbers` | `false` | Recognise `0x…` hex integer literals |
| `m_bCharLiterals` | `false` | Recognise `'x'` character literals |
| `m_bCaseSensitive` | `true` | Case-sensitive keyword matching |

Alternatively use the setter methods:

```cpp
m_lexer->setCPPComments(true);
m_lexer->setCStyleComments(true);
m_lexer->setHexNumbers(true);
m_lexer->caseSensitive(false);
```

#### Query

| Method | Description |
|--------|-------------|
| `std::string getFile() const` | Name of the file currently being parsed |
| `int getLineNumber()` | Current source line number (1-based) |
| `int getColumn()` | Current column (byte offset on current line) |
| `int getTotalLinesParsed()` | Total lines consumed across all input files |
| `const char *getLexemeFromToken(int token)` | Human-readable name for a token value |

#### Error reporting

| Method | Description |
|--------|-------------|
| `virtual void yyerror(const char *msg)` | Report a fatal error (default: print and `exit(-1)`). Override for custom handling. |
| `virtual void yywarning(const char *msg)` | Report a warning (default: print to stdout). |

#### Copy utilities

Helpers for copying raw input — useful in macro-expansion or code-generation parsers:

| Method | Description |
|--------|-------------|
| `void copyToEOF(FILE *out)` | Copy all remaining input to `out` |
| `void copyUntilChar(int end, int nest, FILE *out)` | Copy input to `out` until `end` is seen, tracking `nest` for nesting |
| `void copyUntilChar(int end, int nest, char *buf)` | Same, into a `char` buffer |

---

### `BaseParser`

Base class for the parser. Owns the `LexicalAnalyzer` and `SymbolTable`. Subclass it and implement `yyparse()`.

#### Constructor

```cpp
BaseParser(std::unique_ptr<SymbolTable> symbolTable);
```

Takes ownership of a `SymbolTable`. The `LexicalAnalyzer` is created and assigned to `m_lexer` in the subclass constructor.

#### Public fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `yydebug` | `bool` | `false` | When `true`, calls to `yylog()` produce trace output |
| `yyout` | `FILE*` | `stdout` | Primary output stream (used by parser generators) |
| `yyhout` | `FILE*` | `stdout` | Secondary output stream (header output) |

#### Parsing

| Method | Description |
|--------|-------------|
| `virtual int parseFile(const char *path)` | Open `path`, call `yyparse()`, close. Returns 0 on success. |
| `virtual int parseData(char *text, const char *name, void *userData)` | Parse from an in-memory buffer. `name` appears in error messages. |
| `virtual int yyparse()` | Override this with grammar rules. **Must call `BaseParser::yyparse()` first** to prime the lookahead. |

#### Lookahead and matching

| Member / Method | Description |
|-----------------|-------------|
| `int lookahead` | The current token (set by `match()` and by `BaseParser::yyparse()`). |
| `YYSTYPE yylval` | Semantic value of the current token. |
| `virtual int match(int token)` | Assert `lookahead == token`, advance to next token. Calls `yyerror` on mismatch. |
| `virtual int match()` | `match(lookahead)` — advance unconditionally. |
| `virtual void expected(int token)` | Report an "expected to see X" error for the given token. |

#### Error and log reporting

All methods accept `printf`-style format strings and variadic arguments.

| Method | Description |
|--------|-------------|
| `virtual void yyerror(const char *fmt, ...)` | Error at the current lexer position |
| `virtual void yyerror(const Position &pos, const char *fmt, ...)` | Error at an explicit source position |
| `virtual void yywarning(const char *fmt, ...)` | Warning at the current lexer position |
| `virtual void yywarning(const Position &pos, const char *fmt, ...)` | Warning at an explicit source position |
| `virtual void yylog(const char *fmt, ...)` | Debug trace output — only active when `yydebug == true` |

Error messages use the MS-style format:
```
filename(line) : error near column N: message
```

#### Counters

| Method | Description |
|--------|-------------|
| `unsigned getErrorCount() const` | Total errors reported via `yyerror()` |
| `unsigned getWarningCount() const` | Total warnings reported via `yywarning()` |
| `void addWarningCount(int n)` | Add to warning count (e.g., from a sub-parser) |

#### Symbol table helpers

These delegate to the owned `SymbolTable`:

| Method | Description |
|--------|-------------|
| `SymbolEntry *installSymbol(char *lexeme, SymbolType st = stUndef)` | Insert or return existing entry at the current scope |
| `SymbolEntry *lookupSymbol(char *lexeme)` | Search all scopes from innermost outward |
| `virtual int reportUnreferencedSymbols() const` | Print symbols with `isReferenced == 0` at the current scope level |

---

### `SymbolTable`

A multi-level (scoped) symbol table. Scopes are pushed and popped with `push()`/`pop()`. Usually accessed through `BaseParser`'s helper methods rather than directly.

#### Lookup and install

| Method | Description |
|--------|-------------|
| `SymbolEntry *install(const char *lexeme, SymbolType type)` | Insert a new entry at the current scope level, or return the existing entry if already present |
| `SymbolEntry *lookup(const char *lexeme)` | Search all scope levels from innermost outward; returns `nullptr` if not found |
| `SymbolEntry *reverse_lookup(int ival)` | Find an entry whose `ival` matches the given integer value |

#### Scope management

| Method | Description |
|--------|-------------|
| `void push()` | Enter a new nested scope (e.g., on `{`) |
| `void pop()` | Leave the current scope and discard all symbols at that level |

#### Iteration

```cpp
for (auto it = table.begin_stack(); it != table.end_stack(); ++it) {
    // *it is a std::map<std::string, SymbolEntry>
}
```

#### Diagnostics

| Method | Description |
|--------|-------------|
| `void dumpContents()` | Print all symbols across all scope levels to stdout |
| `int dumpUnreferencedSymbolsAtCurrentLevel()` | Print symbols with `isReferenced == 0` at the current level; returns the count |

---

## Examples

Several example projects are included to help illustrate basic usage of the library.

Provided examples include:

Name | Description
---- | -----------
[json](/examples/json) | A simple JSON parser
[xml](/examples/xml) | A basic XML parser
[bnf](/examples/bnf) | Example of a Yacc-like table-driven LL(1) parser generator
[yaml](/examples/yaml) | A YAML parser
