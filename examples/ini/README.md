# INI Parser example

## Introduction

A standalone parser for classic INI-style configuration files —
`[section]` headers followed by `key = value` pairs — built directly on
`BaseParser`.

## What it parses

| Feature | Example |
|---------|---------|
| Section header | `[server]` |
| String value | `name = "primary"` |
| Bare-word value | `host = localhost` |
| Integer value | `port = 8080` |
| Float value | `timeout = 1.5` |
| Boolean value | `enabled = true` |
| Comments | `; like this` or `# like this` |

## Scoped SymbolTable

This is the only example that uses `SymbolTable::push()`/`pop()`. Each
`[section]` gets its own scope: `IniParser::DoSection()` calls
`m_pSymbolTable->push()` before installing that section's keys, and
`m_pSymbolTable->pop()` once the section is done. That means the same key
name (e.g. `name`) can be reused in different sections — `[server]` and
`[client]` in `sample.ini` both define `name` — without one overwriting
the other, and a key stops being resolvable via `lookupSymbol()` once its
section's scope has been popped. Run with `-v` to see the `push()`/`pop()`
trace.

## Architecture

| File | Role |
|------|------|
| `iniparser.h/cpp` | `IniParser` — recursive-descent parser; `DoSection()`/`DoPair()` mirror the grammar rules |
| `ini.cpp` | Driver: opens a file, calls `parseFile()` |
| `sample.ini` | Demo input with two sections sharing a key name |

## Building

```
c++ -std=c++14 ini.cpp iniparser.cpp \
    ../../lexer.cpp ../../baseparser.cpp ../../symboltable.cpp \
    -o ini
```

Or via the top-level `CMakeLists.txt`/`makefile` (`make ini`).

## Running

```
./ini sample.ini        # parse and print each section/key as it's read
./ini sample.ini -v     # verbose: also trace scope push()/pop() calls
```

`parseFile()` internally `chdir()`s into the target file's directory
before opening it (see `baseparser.cpp`). If you pass a path with a
directory component (e.g. `examples/ini/sample.ini` from the repo root),
use an absolute path, or `cd` into the file's directory first — this is a
library-wide quirk, not specific to `ini`.
