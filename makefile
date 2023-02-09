CC=gcc
CFLAGS= -g -Wall -std=c99
TARGETS= lexer
.PHONY= clean

# Default commands
clean:
	rm $(TARGETS) *.o

# Executable comp
lexer: lexer.o
	$(CC) $(CFLAGS) -o lexer lexer.o

# Object comp
lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o
