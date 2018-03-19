# Compilers
CC=gcc
YACC_CC=yacc
LEX_CC=lex

# Compilers flags
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lm

# Parser sources
YACC_SOURCE=parser.y
LEX_SOURCE=lexer.l

# Parser builds
YACC_BUILD=parser_yacc.c
LEX_BUILD=lexer_yacc.c

# Sources
SOURCES=$(YACC_BUILD) $(LEX_BUILD) shell.c promptline.c parseline.c
OBJECTS=$(SOURCES:.c=.o)
HEADERS=shell.h promptline.h parseline.h

EXECUTABLE=yxsh

all: yacc lex $(HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

yacc: $(YACC_SOURCE)
	$(YACC_CC) -d -o $(YACC_BUILD) $(YACC_SOURCE)

lex: $(LEX_SOURCE)
	$(LEX_CC) -o $(LEX_BUILD) $(LEX_SOURCE)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(YACC_BUILD) $(YACC_BUILD:.c=.h) $(LEX_BUILD)

clear: clean

rebuild: clean all

