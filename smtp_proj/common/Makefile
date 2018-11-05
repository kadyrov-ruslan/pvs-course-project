CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LDFLAGS=-pthread  -lrt

INC = include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig -lpthread -lpcre
TARGET = smtp_client

SRC = log.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: client

client: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -lrt -c -o $@ $< $(CFLAGS) -I $(INC) $(LIB)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: client clean
