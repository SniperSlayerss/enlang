CC := cc
CFLAGS := -Wall -Wextra -O2 -Iinclude -ggdb
SRC :=  src/codegen.c src/parser.c src/lexer.c src/x86_64_nasm.c
OBJ := $(SRC:src/%.c=build/%.o)
TARGET := build/enlang

$(TARGET) :	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf build/
	rm -rf out/
	rm $(TARGET)
