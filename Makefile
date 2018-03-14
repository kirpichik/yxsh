CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS=
SOURCES=shell.c promptline.c parseline.c
OBJECTS=$(SOURCES:.c=.o)
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

