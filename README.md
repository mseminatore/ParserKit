# ParserKit

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
interpreters. I gathered knowledge where and when I could and over time taught
myself the basics of parsing technology. Over many years I've used what I've 
learned to create useful data serialization formats, small scripting languages,
assemblers, and compilers.

Parsing concepts are difficult, and the technology still seems mystical to many.
So I decided that I would share my library. I hope that this library is helpful to 
others.

## How can I learn more about parsing?

I've found that there is no single textbook that clearly presents all that you 
want and need to know about parsing. For those just starting out I would recommend:

1. The [Dragon Book](https://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811/ref=sr_1_1?crid=2M6JW4SE1LFYA&dchild=1&keywords=aho+sethi+ullman&qid=1592861659&sprefix=aho+set%2Caps%2C215&sr=8-1) is one of the classical texts on the subject.
2. Good coverage of the GNU versions of Yacc and Lex is [Flex and Bison](https://www.amazon.com/flex-bison-Text-Processing-Tools/dp/0596155972/ref=sr_1_1?dchild=1&keywords=flex+and+bison&qid=1592863889&sr=8-1)
3. Another good source of information on Yacc and Lex is [The Unix Programming Environment](https://www.amazon.com/Unix-Programming-Environment-Prentice-Hall-Software/dp/013937681X/ref=sr_1_2?crid=2Y5FH064KCR8X&dchild=1&keywords=the+unix+programming+environment&qid=1592861830&sprefix=the+unix+program%2Caps%2C212&sr=8-2)
4. [Modern Compiler Implementation in C](https://www.amazon.com/Modern-Compiler-Implementation-Andrew-Appel/dp/817596071X/ref=sxts_sxwds-bia-wc-p13n1_0?crid=2CGBMJ614Z1BF&cv_ct_cx=modern+compiler+implementation+in+c&dchild=1&keywords=modern+compiler+implementation+in+c&pd_rd_i=817596071X&pd_rd_r=4c3593d8-34db-4796-929b-b84a1ac8cd26&pd_rd_w=Yh0K9&pd_rd_wg=3RWhe&pf_rd_p=1da5beeb-8f71-435c-b5c5-3279a6171294&pf_rd_r=4AF9BXMZ36CGJMNM3FR4&psc=1&qid=1592861915&sprefix=modern+compiler%2Caps%2C214&sr=1-1-70f7c15d-07d8-466a-b325-4be35d7258cc) provides a great overview of different parsing methods as well as 
other aspects of compiler implementation like register allocation and code generation.

# Documentation

The documentation for the library is a work in progress.

## Examples

Several example projects are included to help illustrate some ways to make use 
of the library.

Provided examples include:

Name | Description
---- | -----------
[json](/examples/json) | A simple JSON parser
[xml](/examples/xml) | A basic XML parser
[bnf](/examples/bnf) | Simple Yacc-like table-driven parser generator
[yaml](/examples/yaml) | YAML parser (not yet started)
