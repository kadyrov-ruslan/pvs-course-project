#ifndef _LIST_H_
#define _LIST_H_

#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct node_t {
    char *val;
    struct node_t *next;
} node_t;

void add_first(struct node_t ** head, char *val);
void add_last(node_t * head, char *val);

int remove_first(node_t ** head);
int remove_last(node_t * head);
int remove_by_index(node_t ** head, int n);
int count(node_t *head);


#endif


