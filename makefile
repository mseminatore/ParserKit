TARGET	= libParserKit.lib
LINKER	= cc -o
OBJS	= lexer.o baseparser.o symboltable.o
CFLAGS	= -Wc++11-extensions -std=c++11

$(OBJS):	lexer.cpp baseparser.cpp symboltable.cpp
	$(CC) $< $(CFLAGS)

$(TARGET):	$(OBJS)
	$(LINKER) $(TARGET) $(CFLAGS) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
	