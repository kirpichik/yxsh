# Compilers
CC=gcc

LEX_CC=lex

# Compilers flags
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lm -ll

# Parser sources
YACC_SOURCE=parser.y
LEX_SOURCE=lexer.l
LEX_DISABLED_WARNS=-Wno-unneeded-internal-declaration -Wno-unused-function

# Parser builds
YACC_BUILD=parser_yacc.c
LEX_BUILD=lexer_lex.c

# Sources
SOURCES=shell.c promptline.c
OBJECTS=$(SOURCES:.c=.o)
HEADERS=shell.h promptline.h parser.h

EXECUTABLE=yxsh

all: yacc lex parser_yacc lexer_lex $(HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LEX_BUILD:.c=.o) $(YACC_BUILD:.c=.o) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

parser_yacc:
	$(CC) $(CFLAGS) $(YACC_BUILD) -o $(YACC_BUILD:.c=.o)

lexer_lex:
	$(CC) $(CFLAGS) $(LEX_DISABLED_WARNS) $(LEX_BUILD) -o $(LEX_BUILD:.c=.o)

yacc: $(YACC_SOURCE)
	$(YACC_CC) -d -o $(YACC_BUILD) $(YACC_SOURCE)

lex: $(LEX_SOURCE)
	$(LEX_CC) $(LEX_SOURCE)
	mv lex.yy.c $(LEX_BUILD)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(YACC_BUILD) $(YACC_BUILD:.c=.h) $(LEX_BUILD) $(LEX_BUILD:.c=.o)

clear: clean

rebuild: clean all

