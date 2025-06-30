CC := gcc
CFLAGS := -Wall -Wextra -O2
SRC :=  src/codegen.c src/parser.c src/lexer.c
OBJ := $(SRC:src/%.c=build/%.o)
TARGET := enlang 

$(TARGET) :	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf build/
	rm $(TARGET)
