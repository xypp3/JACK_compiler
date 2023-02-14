CC=gcc
CFLAGS= -g -Wall -std=c99
TARGETS= lexer
.PHONY= clean all

# Default commands
all: $(TARGETS)
	make $(TARGETS) 1>/dev/null

clean:
	rm $(TARGETS) *.o ./lexer_jack_samples/*_my.txt 2>/dev/null

# Executable comp
lexer: lexer.o
	$(CC) $(CFLAGS) -o lexer lexer.o

# Object comp
lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o
