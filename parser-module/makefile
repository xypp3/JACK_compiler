CC=gcc
CFLAGS= -g -Wall -std=c99 
TARGETS= lexer parser
.PHONY= all clean 

# Default commands
all: $(TARGETS)
	make $(TARGETS) 1>/dev/null

clean:
	rm $(TARGETS) *.o ./jack_samples_lexer/*_mine.txt 2>/dev/null

# Executable comp
lexer: lexer.o
	$(CC) $(CFLAGS) -o lexer lexer.o

parser: parser.c
	$(CC) $(CFLAGS) -D TEST -o parser lexer.h lexer.c parser.h parser.c 

# Object comp
lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o
