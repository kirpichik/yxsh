CC=g++
CFLAGS=-c -Wall -std=c++98
LDFLAGS=
SOURCES=shell.cpp promptline.cpp parseline.cpp command.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=yxsh

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

clear: clean

rebuild: clean all

