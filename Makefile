# Compiler
CC=gcc

ifndef BISON_CC
BISON_CC=bison
endif

# Compiler flags
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lm

# Bison parser source
BISON_SOURCE=parser.y

# Parser pre-builds
BISON_PRE_BUILD=parser.c

# Sources
SOURCES=$(YACC_PRE_BUILD) shell.c promptline.c
OBJECTS=$(SOURCES:.c=.o)
HEADERS=shell.h promptline.h parser.h

EXECUTABLE=yxsh

all: bison $(HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LEX_BUILD:.c=.o) $(YACC_BUILD:.c=.o) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

bison: $(BISON_SOURCE)
	$(BISON_CC) -d -o $(BISON_PRE_BUILD) $(BISON_SOURCE)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BISON_PRE_BUILD) $(BISON_PRE_BUILD:.c=.h)

clear: clean

rebuild: clean all

