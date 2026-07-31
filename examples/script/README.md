# Script Parser example

## Introduction

A tiny statement language — variable assignment, `print`, and
`#include`-style file inclusion — built directly on `BaseParser`. It
exists mainly to demonstrate three `ParserKit` features that none of the
other examples touch.

## What it does

| Statement | Example | Notes |
|-----------|---------|-------|
| Assignment | `x = 1 + 2 * 3;` | Stores a `double` in the flat symbol table |
| Print | `print x;` | Evaluates an expression and prints it |
| Include | `include "lib.script";` | Splices another file's statements in at this point |

See `demo.script` (includes `lib.script`) and `errors.script` (deliberately
broken, for the error-recovery demo below).

## File inclusion

`include "path";` calls `m_lexer->pushFile(path)` mid-parse. Nothing else
is needed: `LexicalAnalyzer::specialTokens()` already pops the file stack
and transparently resumes the includer's stream when the included file
hits EOF, so nested `include`s "just work". `demo.script` includes
`lib.script`, which defines `pi` and `e`, and both are then used back in
`demo.script`.

## Inline scripts (`-e`)

`script -e "x = 2; print x;"` runs the driver through `parseData()`
instead of `parseFile()` — the first example to exercise the in-memory
parsing path. See `script.cpp`.

## Error recovery

The library's default `yyerror()` prints and calls `exit(-1)` on the very
first error — fine for a one-shot parse, but it means a script with two
unrelated typos only ever reports the first one. `ScriptParser::yyerror()`
overrides this: it reports the error, then skips tokens up to the next
`;` (panic-mode recovery) so parsing continues. Run `script errors.script`
to see every error in the file reported in one pass instead of the
process dying on the first.

## Notable ParserKit quirks worked around here

- **Sign-folding in `yylex()`.** The stock `LexicalAnalyzer::yylex()`
  always folds a leading `+`/`-` into the number that follows as its
  sign — so `2 + 3` would tokenize as two integers with the `+` silently
  swallowed, never as a binary operator. `ScriptLexer` (in
  `scriptparser.cpp`) overrides `yylex()` to return `+`/`-` as their own
  single-character tokens instead; negative literals like `-5` are
  handled by the grammar's own unary-minus production in `DoFactor()`.
- **Identifiers are auto-installed.** The lexer installs *any* identifier
  it sees into the symbol table on first sight (with `type == stUndef`),
  so it always has a `SymbolEntry` to point `yylval.sym` at. That means
  `!lookupSymbol(name)` can never detect an undefined variable — the
  entry already exists. `DoFactor()`'s `TV_ID` case instead checks
  `sym->type == stUndef` to tell "referenced but never assigned" apart
  from a real value.

## Architecture

| File | Role |
|------|------|
| `scriptparser.h/cpp` | `ScriptParser` (+ `ScriptLexer`) — recursive-descent parser and evaluator; `DoStmt()`/`DoExpr()`/`DoTerm()`/`DoFactor()` mirror the grammar rules |
| `script.cpp` | Driver: `parseFile()` or `-e` + `parseData()` |
| `demo.script` / `lib.script` | Demo input showing file inclusion |
| `errors.script` | Demo input with multiple independent errors |

## Building

```
c++ -std=c++14 script.cpp scriptparser.cpp \
    ../../lexer.cpp ../../baseparser.cpp ../../symboltable.cpp \
    -o script
```

Or via the top-level `CMakeLists.txt`/`makefile` (`make script`).

## Running

```
./script demo.script            # parse and run a script file
./script demo.script -v         # verbose: also trace parser rule calls
./script -e "x = 2; print x;"   # parse and run an inline script
./script errors.script          # see every error reported, not just the first
```

`parseFile()` internally `chdir()`s into the target file's directory
before opening it. If you pass a path with a directory component, use an
absolute path, or `cd` into the file's directory first — this is a
library-wide quirk, not specific to `script`.
