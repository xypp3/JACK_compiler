#compiler makefile
SRC:= $(shell find ./src -name '*.c')
OBJS:= $(SRC:%=./build/%.o)
TARGET:= compile
CDEF:= -D TEST -D TEST_PARSER -D TEST_SYMBOL

CC:= gcc
CFLAGS:= -g -Wall -std=c99 

# Default commands
./build/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

./build/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CDEF) -c $< -o $@ 

#.PHONY
.PHONY:= clean 

clean:
	rm -r ./build
