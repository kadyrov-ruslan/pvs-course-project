CC=gcc
CFLAGS=-Wall -Werror -ggdb3 -D_POSIX_C_SOURCE -std=c99
INC=-Iinclude -Iproto -I../common/include
LIB=-lconfig -lpcre

OBJ_DIR=obj
BIN_DIR=bin
SOURCES=$(wildcard src/*.c)
SOURCES_TEST=$(SOURCES) test/test.c
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_TEST=$(SOURCES_TEST:.c=.o)
MAIN_OBJECTS=$(filter-out $(OBJ_DIR)/test.o, $(wildcard $(OBJ_DIR)/*.o))
TEST_OBJECTS=$(OBJ_DIR)/pattern.o $(OBJ_DIR)/test.o
TARGET=smtp_server

.PHONY: all
all: server

.PHONY: server
server: mkdir autogen $(OBJECTS)
	$(CC) $(MAIN_OBJECTS) $(LIB) -o $(BIN_DIR)/$(TARGET)

.PHONY: tests
tests: test_units

.PHONY: test_units
test_units: mkdir autogen $(OBJECTS_TEST)
	$(CC) $(TEST_OBJECTS) $(LIB) -lcunit -o $(BIN_DIR)/$(TARGET)_test
	$(BIN_DIR)/$(TARGET)_test
	rm $(BIN_DIR)/$(TARGET)_test

.c.o:
	$(CC) -c $(CFLAGS) $(INC) $< -o $(OBJ_DIR)/$(notdir $@)

.PHONY: autogen
autogen:
	cd proto; autogen smtp.tpl
	$(CC) -c -w -fno-builtin-exit proto/*.c $< -o $(OBJ_DIR)/$(notdir $@).o

.PHONY: mkdir
mkdir:
	mkdir -p ./$(OBJ_DIR)
	mkdir -p ./$(BIN_DIR)

.PHONY: clean
clean:
	rm -f proto/*.c proto/*.h
	rm -rf $(OBJ_DIR)*
	rm -rf $(BIN_DIR)*
