add_executable(yaml
    yaml.cpp
    yamlparser.cpp
    yamllexer.cpp
    yamlvalue.cpp
)

target_link_libraries(yaml PRIVATE ParserKit)
