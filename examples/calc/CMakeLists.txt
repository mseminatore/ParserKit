add_executable(calc
    calc.cpp
    calcparser.cpp
)

target_link_libraries(calc PRIVATE ParserKit)
