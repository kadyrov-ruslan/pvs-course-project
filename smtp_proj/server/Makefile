CC = gcc
CFLAGS = -Wall -Werror -std=c99

INC = -I include -I ../common/include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig
TARGET = smtp_server

SRC = server.c conn.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: server

server: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -lrt -c -o $@ $< $(CFLAGS) -I $(INC) $(LIB)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: server clean
