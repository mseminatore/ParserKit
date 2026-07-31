add_executable(json
    json.cpp
    jsonparser.cpp
    jsonvalue.cpp
)

target_link_libraries(json PRIVATE ParserKit)
