# Calculator example

## Introduction

A four-function calculator built directly on `BaseParser`. Unlike
`examples/bnf` — which *generates* a `PrattParser<T>` from a `.y` grammar
file (see `examples/bnf/calc.y` and `examples/bnf/tableparser.h`) — this
parser implements Pratt-style precedence climbing by hand, directly
against `BaseParser::lookahead`/`match()`. It's worth reading the two
side by side: this is what the generator in `examples/bnf` is automating.

## Supported operators

| Operator | Associativity | Precedence |
|----------|----------------|------------|
| `+` `-` | left | 1 (lowest) |
| `*` `/` | left | 2 |
| `^` (power) | right | 3 (highest) |
| unary `-` | — | binds to the immediate primary |

Integer, float, and `0x...` hex literals are all supported.

## How precedence climbing works here

`CalcParser::parseExpr(minPrec)` parses the tightest-binding chain of
operators whose precedence is at least `minPrec`:

```cpp
double CalcParser::parseExpr(int minPrec)
{
    double left = parsePrimary();

    while (isBinOp(lookahead) && precedence(lookahead) >= minPrec)
    {
        int op = lookahead;
        int nextMinPrec = isRightAssoc(op) ? precedence(op) : precedence(op) + 1;

        match(op);
        double right = parseExpr(nextMinPrec);
        left = apply(op, left, right);
    }

    return left;
}
```

For a left-associative operator, the recursive call raises `minPrec` by
one so the loop — not the recursion — picks up the next operator at the
same level (left-to-right grouping). For a right-associative operator
(`^`), `minPrec` stays the same, so the recursive call *can* re-enter the
same level (right-to-left grouping) — e.g. `2 ^ 3 ^ 2` evaluates as
`2 ^ (3 ^ 2)` = 512, not `(2 ^ 3) ^ 2` = 64.

## Notable ParserKit quirk worked around here

The stock `LexicalAnalyzer::yylex()` always folds a leading `+`/`-` into
the number that follows it as its sign — so `2 + 3` would tokenize as two
integers with the `+` silently swallowed, never as a binary operator.
`CalcLexer` (in `calcparser.cpp`) overrides `yylex()` to return `+`/`-`
as their own single-character tokens instead; negative literals like
`-5` are handled by the grammar's own unary-minus production in
`parsePrimary()`.

## Architecture

| File | Role |
|------|------|
| `calcparser.h/cpp` | `CalcParser` (+ `CalcLexer`) — hand-written precedence-climbing parser/evaluator |
| `calc.cpp` | Driver: REPL over stdin, or `parseFile()` on a given file |
| `sample.calc` | Demo input covering every operator |

## Building

```
c++ -std=c++14 calc.cpp calcparser.cpp \
    ../../lexer.cpp ../../baseparser.cpp ../../symboltable.cpp \
    -o calc
```

Or via the top-level `CMakeLists.txt`/`makefile` (`make calc`).

## Running

```
./calc                 # REPL: evaluate one expression per line from stdin
./calc sample.calc      # evaluate every expression in a file
./calc sample.calc -v   # verbose: also trace parser rule calls
```

`parseFile()` internally `chdir()`s into the target file's directory
before opening it. If you pass a path with a directory component, use an
absolute path, or `cd` into the file's directory first — this is a
library-wide quirk, not specific to `calc`.
