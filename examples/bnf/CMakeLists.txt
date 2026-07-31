add_executable(bnf
    bnf.cpp
    bnfparser.cpp
    bnflexer.cpp
    tableparser.cpp
)

target_link_libraries(bnf PRIVATE ParserKit)
