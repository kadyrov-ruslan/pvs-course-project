CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LDFLAGS= -lresolv  -lrt -lcunit

#INC = -I include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig -lresolv
TARGET = smtp_client_test

SRC = smtp_client_test.c ../../src/smtp_client.c ../../src/msg.c ../../src/mx_utils.c ../../src/log.c ../../src/list.c ../../../common/src/string_utils.c ../../../common/src/dir_utils.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: tests

tests: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -lrt -c -o $@ $< $(CFLAGS) -I $(INC) $(LIB)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: tests clean
