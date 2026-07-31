add_executable(script
    script.cpp
    scriptparser.cpp
)

target_link_libraries(script PRIVATE ParserKit)
