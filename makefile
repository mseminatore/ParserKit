TARGET	= libParserKit.lib
OBJS	= lexer.o baseparser.o symboltable.o
CXX	= c++
CFLAGS	= -Wc++11-extensions -std=c++11
AR	= ar rcs

%.o:	%.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

$(TARGET):	$(OBJS)
	$(AR) $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
