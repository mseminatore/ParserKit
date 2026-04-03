# BNF Parser example 

## Introduction
This is an advanced demo that uses `ParserKit` to create a Yacc-like 
table-driven parser generator. The tool consumes an input file using 
Yacc syntax and outputs the code for a parser. As with Yacc you can 
embed semantic actions in the grammar input file.

If you are not familiar with Yacc you may want to read about it before
jumping into this example.

>This example is very complicated. It is primarily intended to show more
>of the range of what can be done with the `ParserKit` library.

## Grammar files included

| File | Mode | Description |
|------|------|-------------|
| `calc.y` | Pratt | Four-function calculator with operator precedence |
| `json.y` | LL(1) | JSON parser (objects, arrays, strings, numbers, keywords) |
| `yaml.y` | LL(1) | YAML parser (block + flow mappings/sequences, all scalar types) |

## More grammar ideas

These formats would make good additional examples and highlight different
aspects of the tool:

| Grammar | Mode | Why it's interesting |
|---------|------|----------------------|
| `ini.y` | LL(1) | Classic `[section]` + `key=value` format; minimal grammar, great for beginners |
| `csv.y` | LL(1) | RFC 4180 fields and records; shows quoted-field disambiguation |
| `toml.y` | LL(1) | Typed values, arrays, inline tables; richer LL(1) than ini |
| `dot.y` | LL(1) | GraphViz DOT language; node/edge declarations, attribute lists |
| `logfmt.y` | LL(1) | `key=value` structured log format used by many Go/cloud services |
| `calc2.y` | Pratt | Extended calculator: unary minus, `^` (right-assoc power), `sin`/`cos` calls |
| `regex.y` | Pratt | Regex subset: `\|` (alternation), concat, `*` / `+` / `?` as Pratt operators |

## Operator Precedence

The generator supports two output modes depending on whether the grammar
declares operator precedence.

### LL(1) mode (no precedence declarations)

When no `%left`/`%right`/`%nonassoc` directives are present, the tool
generates a pure LL(1) table-driven parser (`TableParser` subclass).
This is the original behaviour.

Operator precedence in LL(1) requires manual grammar factoring, which
was previously a significant challenge:

```yacc
expr: term expr_tail { $$ = $1; }
    ;
expr_tail:
    | '+' term expr_tail { $< = $< + $2; }
    ;
term: factor term_tail
    ;
```

### Pratt mode (%left / %right / %nonassoc)

When one or more precedence directives are present the tool generates a
**Pratt / Top-Down Operator Precedence** parser (`PrattParser<T>`
subclass). This approach was introduced by Vaughan Pratt in 1973 and
is used in many modern language implementations.

With Pratt mode the same calculator grammar becomes:

```yacc
%left  '+' '-'
%left  '*' '/'

%%

calc: expr   { printf("Result is %f\n", $1); }
    ;

expr: NUMBER
    | '(' expr ')'  { $$ = $2; }
    | expr '+' expr { $$ = $1 + $3; }
    | expr '-' expr { $$ = $1 - $3; }
    | expr '*' expr { $$ = $1 * $3; }
    | expr '/' expr { $$ = $1 / $3; }
    ;
```

Each `%left` line defines a group of operators at the same precedence
level. Lines listed later have higher precedence than earlier lines.
Use `%right` for right-associative operators (e.g. exponentiation,
assignment) and `%nonassoc` for operators that cannot be chained.

#### How it works

Each operator is assigned a pair of binding powers `{lbp, rbp}`:
- **Left-assoc** (`%left`):  `lbp == rbp` — the loop stops before
  re-entering the same level, giving left-to-right grouping.
- **Right-assoc** (`%right`): `rbp == lbp - 1` — the recursive call
  can re-enter the same level, giving right-to-left grouping.

The generated `parseExpr(minBP)` algorithm:
1. Consume the current token and call `nud()` to get the left value.
2. While the next token's `lbp > minBP`, consume the operator, call
   `parseExpr(rbp)` for the right operand, then call `led()` to
   combine them.

The generated code consists of:
- `initPrattTable()` — populates `m_infixBP` from the declarations.
- `nud(token, val)` — handles atoms (`NUMBER`) and prefix forms
  (`'(' expr ')'`), generated from non-infix grammar productions.
- `led(op, left, right)` — handles binary operators, generated from
  `expr OP expr` productions.
- `yyparse()` — calls `parseExpr(0)` and executes the start rule action.
