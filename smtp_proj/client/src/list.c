#include "../include/list.h"

void add_first(struct node_t **head, char *val)
{
    struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
    new_node->val = malloc(strlen(val));
    strcpy(new_node->val, val);
    new_node->next = *head;
    *head = new_node;
}

void add_last(node_t * head, char *val)
{
    node_t *current = head;
    // while (current->next != NULL)
    // {
    //     current = current->next;
    // }

    // /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    strcpy(current->next->val, val);
    current->next->next = NULL;
    printf("after while last \n");
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

int remove_last(node_t * head)
{
    /* if there is only one item in the list, remove it */
    if (head->next == NULL)
    {
        free(head);
        return 1;
    }

    /* get to the second to last node in the list */
    node_t *current = head;
    while (current->next->next != NULL)
    {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    free(current->next);
    current->next = NULL;
    return 1;
}

int remove_by_index(node_t ** head, int n)
{
    int i = 0;
    node_t *current = *head;
    node_t *temp_node = NULL;

    if (n == 0)
        return remove_first(head);

    for (i = 0; i < n - 1; i++)
    {
        if (current->next == NULL)
            return -1;

        current = current->next;
    }

    temp_node = current->next;
    current->next = temp_node->next;
    free(temp_node);

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