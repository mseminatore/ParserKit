# YAML Example

A standalone YAML parser built directly on `BaseParser` and a custom
`YAMLLexer` subclass.  Mirrors the structure of the `json/` example.

## What it parses

A useful subset of YAML 1.2:

| Feature | Example |
|---------|---------|
| Block mapping | `key: value` |
| Nested block mapping | `parent:\n  child: val` |
| Block sequence | `- item` |
| Flow mapping | `{k: v, k2: v2}` |
| Flow sequence | `[a, b, c]` |
| Quoted string | `"hello world"` |
| Bare-word scalar | `hello` |
| Integer | `42` |
| Float | `3.14` |
| Boolean | `true` / `yes` / `on` (case-insensitive) |
| Null | `null` / `~` |

## Architecture

| File | Role |
|------|------|
| `yamllexer.h/cpp` | `YAMLLexer` — extends `LexicalAnalyzer`; manages the INDENT/DEDENT stack and emits the `TV_KEY` token for mapping keys |
| `yamlvalue.h/cpp` | `YAMLValue` — variant node type (Null, Bool, Int, Float, String, Sequence, Mapping) |
| `yamlparser.h/cpp` | `YAMLParser` — recursive-descent parser; `Do*()` methods mirror the grammar rules |
| `yaml.cpp` | Driver: opens a file, calls `parseFile()`, dumps the result |
| `test.yaml` | Demo input covering all supported features |

### KEY token trick

YAML block mappings are LL(2) if bare-word keys share the `SCALAR` token
with plain-scalar values, because the parser cannot decide which production
to use without looking two tokens ahead.  `YAMLLexer` resolves this by
emitting `TV_KEY` (not `TV_SCALAR`) when a bare word is immediately followed
by `:` in block context, making the grammar unambiguously LL(1).

## Building

Copy or symlink the library headers/sources into the project, then:

```
c++ -std=c++11 yaml.cpp yamlparser.cpp yamllexer.cpp yamlvalue.cpp \
    ../../lexer.cpp ../../baseparser.cpp ../../symboltable.cpp \
    -o yaml
```

## Running

```
./yaml test.yaml        # parse and print the value tree
./yaml test.yaml -v     # verbose: trace rule calls
```

## Known limitations

- Only double-quoted strings; single-quoted and block scalars are not handled.
- Only decimal integers; hex (`0x…`) is not recognised.
- No anchors (`&`), aliases (`*`), or tags (`!`).
- No multi-document streams (`---` / `...`).