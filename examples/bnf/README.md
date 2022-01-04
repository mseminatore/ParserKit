# BNF Parser example 

## Introduction
This is an advanced demo that uses `ParserKit` to create a Yacc-like 
table-driven parser generator. The tool consumes an input file using 
Yacc syntax and outputs the code for an LL(1) table-driven parser. As
with Yacc you can embed semantic actions in the grammar input file.

If you are not familiar with Yacc you may want to read about it before
jumping into this example.

>This example is very complicated. It is primarily intended to show more
>of the range of what can be done with the `ParserKit` library.

There are some significant challenges with LL(1) parsers and the order
in which the parse tree is evaluated. This becomes obvious when parsing
arithmetic expressions. In partciular, it is very difficult to define an 
LL(1) parser that follows normal operator precedence rules.
