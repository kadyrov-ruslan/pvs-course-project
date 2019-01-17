CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LDFLAGS=-lresolv  -lrt

INC = -I include -I ../common/include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig -lresolv
TARGET = smtp_client

SRC = client.c scheduler.c domain_proc.c mx_utils.c smtp_client.c msg.c log.c list.c client-fsm.c ../../common/src/string_utils.c ../../common/src/dir_utils.c
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
