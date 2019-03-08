CC = gcc
CFLAGS = -Wall -Werror -std=gnu99 -ggdb3
LDFLAGS= -lresolv  -lrt -lcunit

#INC = -I include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig -lresolv
TARGET = smtp_client_system_test

SRC = main.c ../../../src/smtp_client.c ../../../src/msg.c ../../../src/mx_utils.c ../../../src/log.c ../../../src/list.c ../../../../common/src/string_utils.c ../../../../common/src/dir_utils.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: system_test smtp_client
	./smtp_client_system_test
	/home/dev/pvs-course-project/smtp_proj/client/./smtp_client client.cfg /home/dev/pvs-course-project/smtp_proj/client/tests/system_tests/logs/

system_test: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -lrt -c -o $@ $< $(CFLAGS) -I $(INC) $(LIB)

smtp_client: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -lrt -c -o $@ $< $(CFLAGS) -I $(INC) $(LIB)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: tests clean
