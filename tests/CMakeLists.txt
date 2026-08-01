# project() only enables the CXX language; force this vendored C source to
# build with the CXX compiler rather than silently being dropped for lack
# of a configured C toolchain.
set_source_files_properties(testy/test_main.c PROPERTIES LANGUAGE CXX)

add_executable(parserkit_tests
    testy/test_main.c
    test_runner.cpp
    test_symboltable.cpp
    test_lexer.cpp
    test_baseparser.cpp
)

target_link_libraries(parserkit_tests PRIVATE ParserKit)
target_include_directories(parserkit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME ParserKitTests COMMAND parserkit_tests)
