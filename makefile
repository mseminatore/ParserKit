TARGET	= libParserKit.lib
OBJS	= lexer.o baseparser.o symboltable.o
CXX	= c++
CFLAGS	= -Wc++11-extensions -std=c++11
CFLAGS14 = -Wc++11-extensions -std=c++14
AR	= ar rcs

EXAMPLE_INCLUDES = -I.

# Example source files
JSON_SRCS   = examples/json/json.cpp examples/json/jsonparser.cpp examples/json/jsonvalue.cpp
XML_SRCS    = examples/xml/xml.cpp examples/xml/xmlparser.cpp
BNF_SRCS    = examples/bnf/bnf.cpp examples/bnf/bnfparser.cpp examples/bnf/bnflexer.cpp examples/bnf/tableparser.cpp
YAML_SRCS   = examples/yaml/yaml.cpp examples/yaml/yamlparser.cpp examples/yaml/yamllexer.cpp examples/yaml/yamlvalue.cpp
INI_SRCS    = examples/ini/ini.cpp examples/ini/iniparser.cpp
SCRIPT_SRCS = examples/script/script.cpp examples/script/scriptparser.cpp
CALC_SRCS   = examples/calc/calc.cpp examples/calc/calcparser.cpp

EXAMPLES   = json xml bnf yaml ini script calc

.PHONY: all examples clean $(EXAMPLES)

all: $(TARGET) examples

%.o:	%.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

$(TARGET):	$(OBJS)
	$(AR) $(TARGET) $(OBJS)

examples: $(EXAMPLES)

json: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(JSON_SRCS) $(TARGET) -o json

xml: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(XML_SRCS) $(TARGET) -o xml

bnf: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(BNF_SRCS) $(TARGET) -o bnf

yaml: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(YAML_SRCS) $(TARGET) -o yaml

ini: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(INI_SRCS) $(TARGET) -o ini

script: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(SCRIPT_SRCS) $(TARGET) -o script

calc: $(TARGET)
	$(CXX) $(CFLAGS14) $(EXAMPLE_INCLUDES) $(CALC_SRCS) $(TARGET) -o calc

clean:
	rm -f $(OBJS) $(TARGET) $(EXAMPLES)
