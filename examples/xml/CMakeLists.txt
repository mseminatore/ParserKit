add_executable(xml
    xml.cpp
    xmlparser.cpp
)

target_link_libraries(xml PRIVATE ParserKit)
