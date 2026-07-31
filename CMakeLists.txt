cmake_minimum_required(VERSION 3.14)
project(ParserKit CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress MSVC secure CRT deprecation warnings project-wide
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ParserKit static library
add_library(ParserKit STATIC
    lexer.cpp
    baseparser.cpp
    symboltable.cpp
)

target_include_directories(ParserKit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(ParserKit PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wc++11-extensions>
)

# Example executables require C++14 (std::make_unique)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(examples/json)
add_subdirectory(examples/xml)
add_subdirectory(examples/bnf)
add_subdirectory(examples/yaml)
