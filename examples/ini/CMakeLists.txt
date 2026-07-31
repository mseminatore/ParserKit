add_executable(ini
    ini.cpp
    iniparser.cpp
)

target_link_libraries(ini PRIVATE ParserKit)
