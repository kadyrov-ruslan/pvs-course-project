CC=gcc
CFLAGS=-Wall -Werror -D_POSIX_C_SOURCE -std=c99
INC=-Iinclude -I../common/include
LIB=-lconfig -lpcre
LIB_TEST=-lconfig -lpcre -lcunit

OBJ_DIR=obj
BIN_DIR=bin
SOURCES=$(wildcard src/*.c)
SOURCES_TEST=$(wildcard src/*.c) test/test.c
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_TEST=$(SOURCES_TEST:.c=.o)
MAIN_OBJECTS=$(filter-out $(OBJ_DIR)/test.o, $(wildcard $(OBJ_DIR)/*.o))
TEST_OBJECTS=$(OBJ_DIR)/pattern.o $(OBJ_DIR)/test.o
TARGET=smtp_server

.PHONY: all
all: server

.PHONY: server
server: mkdir | $(OBJECTS)
	$(CC) $(MAIN_OBJECTS) $(LIB) -o $(BIN_DIR)/$(TARGET)

.PHONY: tests
tests: test_units

.PHONY: test_units
test_units: mkdir | $(OBJECTS_TEST)
	$(CC) $(TEST_OBJECTS) $(LIB_TEST) -o $(BIN_DIR)/$(TARGET)_test
	$(BIN_DIR)/$(TARGET)_test
	rm $(BIN_DIR)/$(TARGET)_test

.c.o:
	$(CC) -c $(CFLAGS) $(INC) $< -o $(OBJ_DIR)/$(notdir $@)

.PHONY: mkdir
mkdir:
	mkdir -p ./$(OBJ_DIR)
	mkdir -p ./$(BIN_DIR)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)*
	rm -rf $(BIN_DIR)*
