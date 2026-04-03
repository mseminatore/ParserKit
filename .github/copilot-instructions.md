# ParserKit – Copilot Instructions

## Git

Do not stage or commit files on my behalf. I review all changes before check-in.

## What This Library Does

ParserKit is a C++11 library for building **top-down recursive-descent predictive parsers** for LL(1) grammars. It provides three reusable components that work together:

- **`LexicalAnalyzer`** (`lexer.h/cpp`) – tokenizes input from a file or in-memory buffer
- **`BaseParser`** (`baseparser.h/cpp`) – base class that drives parsing; owns the lexer and symbol table
- **`SymbolTable`** (`symboltable.h/cpp`) – multi-level (scoped) symbol table with `push()`/`pop()` for scope management

## Build

```bash
make          # builds libParserKit.lib
make clean    # removes object files and library
```

C++11 is required (`-std=c++11`). Each example under `examples/` has its own Visual Studio `.sln`/`.vcxproj` and must be built separately from the library.

There is no test suite.

## Architecture & Data Flow

```
Input (file or string)
    └─► LexicalAnalyzer::yylex()   → produces tokens
            └─► BaseParser::yyparse()   → drives grammar rules
                    ├── match() / lookahead
                    ├── yyerror() / yywarning() / yylog()
                    └── SymbolTable (install / lookup / scope push/pop)
```

`BaseParser` owns both the lexer (`std::unique_ptr<LexicalAnalyzer> m_lexer`) and symbol table (`std::unique_ptr<SymbolTable> m_pSymbolTable`). The constructor receives a pre-constructed `SymbolTable` via `unique_ptr`.

## Extending the Library

### Minimal parser subclass

```cpp
// 1. Define user tokens starting from TV_USER
enum { TV_TRUE = TV_USER, TV_FALSE, TV_NULL };

// 2. Null-terminated token table mapping lexemes → token values
TokenTable _tokenTable[] = {
    { "true",  TV_TRUE  },
    { "false", TV_FALSE },
    { nullptr, TV_DONE  }   // sentinel
};

// 3. Subclass BaseParser
class MyParser : public BaseParser {
public:
    MyParser() : BaseParser(std::make_unique<SymbolTable>()) {
        m_lexer = std::make_unique<LexicalAnalyzer>(_tokenTable, this, &yylval);
    }

    int yyparse() override;
    void DoSomething();   // grammar rule methods named Do<X>()
};

// 4. yyparse() MUST call BaseParser::yyparse() first to prime lookahead
int MyParser::yyparse() {
    BaseParser::yyparse();
    DoSomething();
    return 0;
}
```

### Minimal custom lexer subclass

Override `specialTokens()` to handle multi-character punctuation; set feature flags in the constructor:

```cpp
class MyLexer : public LexicalAnalyzer {
public:
    MyLexer(TokenTable *tt, BaseParser *p, YYSTYPE *v)
        : LexicalAnalyzer(tt, p, v)
    {
        m_bCStyleComments = true;   // /* ... */
        m_bCPPComments    = true;   // // ...
        m_bCharLiterals   = true;   // 'x'
        m_bHexNumbers     = true;   // 0x...
    }

    int specialTokens(int chr) override;  // handle %%  %{  etc.
};
```

Lexer feature flags: `m_bUnixComments`, `m_bCPPComments`, `m_bCStyleComments`, `m_bASMComments`, `m_bHexNumbers`, `m_bCharLiterals`, `m_bCaseSensitive`.

## Key Conventions

- **Grammar rule methods are named `Do<X>()`** – e.g., `DoObject()`, `DoArray()`, `DoValue()`, `DoRules()`.
- **Single-character tokens** use their ASCII value directly: `match('{')`, `match(':')`.
- **`lookahead`** holds the current token; `match(token)` consumes it and advances the lexer.
- **`yylval`** holds the semantic value of the current token (`YYSTYPE` union: `ival`, `fval`, `char_val`, `sym`, `ptt`).
- When a token is `TV_STRING` or `TV_ID`, `yylval.sym` points to the `SymbolEntry` in the symbol table (lexeme is in `yylval.sym->lexeme`).
- **`yydebug = true`** enables trace output via `yylog()` calls scattered through grammar methods.
- Error messages use MS-style format: `filename(line) : error near column N: message`.
- Ownership is managed with `std::unique_ptr` throughout; no raw `new`/`delete`.

## Predefined Token Values

```
TV_ERROR    – lexer error
TV_DONE     – end of input
TV_INTVAL   – integer literal  (yylval.ival)
TV_FLOATVAL – float literal    (yylval.fval)
TV_CHARVAL  – char literal     (yylval.char_val)
TV_STRING   – string literal   (yylval.sym->lexeme)
TV_ID       – identifier       (yylval.sym->lexeme)
TV_USER     – start of user-defined tokens
```

## SymbolTable

- `install(lexeme, type)` – inserts or returns existing entry at the current scope level
- `lookup(lexeme)` – searches all scope levels from innermost outward
- `reverse_lookup(ival)` – finds an entry by integer value
- `push()` / `pop()` – enter/leave a nested scope
- `dumpUnreferencedSymbolsAtCurrentLevel()` – reports symbols with `isReferenced == 0`
- `SymbolType` starts at `stUndef`; user types start at `stUser`

## Examples

| Directory | Description |
|-----------|-------------|
| `examples/json/` | JSON parser (`JSONParser` + `JSONValue` AST) |
| `examples/xml/`  | XML parser (`XMLParser`) |
| `examples/bnf/`  | Yacc-style table-driven parser generator; also shows custom lexer subclass (`BNFLexer`) and Pratt-style operator precedence |
