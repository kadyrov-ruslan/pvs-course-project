#include "letter.h"

#include <stdlib.h>

letter_t *letter_create()
{
    letter_t *letter = malloc(sizeof(letter_t));
    letter->mail_from = calloc(255, sizeof(char));
    letter->rcpt_to = calloc(255, sizeof(char));
    letter->rcpt_username = calloc(255, sizeof(char));
    letter->rcpt_domain = calloc(255, sizeof(char));
    letter->body = calloc(1024 * 1024 * 20, sizeof(char));
    letter->file = NULL;
    return letter;
}

void letter_free(letter_t *letter)
{
    free(letter->mail_from);
    free(letter->rcpt_to);
    free(letter->rcpt_username);
    free(letter->rcpt_domain);
    free(letter->body);
    free(letter);
}
