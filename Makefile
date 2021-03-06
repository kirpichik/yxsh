# Compiler
CC=gcc
BISON_CC=bison

# Compiler flags
INCLUDES=
READLINE_LIB=-lcurses -lreadline
CFLAGS=-c -Wall -std=gnu99 $(INCLUDES)
LDFLAGS=-lm $(READLINE_LIB)

# Bison parser source
BISON_SOURCE=parser.y

# Parser pre-builds
BISON_PRE_BUILD=parser.c

# Sources
SOURCES=$(BISON_PRE_BUILD)\
				shell.c\
				executor.c\
				builtin.c\
				tasks.c
HEADERS=shell.h\
				parseline.h\
				executor.h\
				builtin.h\
				tasks.h

# Compiler output
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=yxsh

all: bison $(HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) $< -o $@

bison: $(BISON_SOURCE)
	$(BISON_CC) -o $(BISON_PRE_BUILD) $(BISON_SOURCE)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BISON_PRE_BUILD) $(BISON_PRE_BUILD:.c=.h)

clear: clean

rebuild: clean all

