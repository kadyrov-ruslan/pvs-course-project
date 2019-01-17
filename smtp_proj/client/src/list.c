#include "../include/list.h"

void add_first(struct node_t **head, char *val)
{
    struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
    new_node->val = malloc(strlen(val));
    strcpy(new_node->val, val);
    new_node->next = *head;
    *head = new_node;
}

int remove_first(node_t ** head)
{
    node_t *next_node = NULL;
    if (*head == NULL)
        return -1;

    next_node = (*head)->next;
    free((*head)->val);
    free(*head);
    *head = next_node;

    return 1;
}

int count(node_t *head)
{
    int i = -1;
    while (head != NULL)
    {
        head = head->next;
        i++;
    }
    return i;
}