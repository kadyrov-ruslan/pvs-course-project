CC=gcc
CFLAGS=-Wall -Werror -std=c99
INC=-Iinclude -I../common/include
LIB=-lconfig -lresolv

OBJ_DIR=obj
BIN_DIR=bin
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
TARGET=smtp_server

.PHONY: all
all: mkdir | server

.PHONY: server
server: $(OBJECTS)
	$(CC) $(OBJ_DIR)/*.o $(LIB) -o $(BIN_DIR)/$(TARGET)

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
