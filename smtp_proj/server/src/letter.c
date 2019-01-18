#include "letter.h"

#include <stdlib.h>

letter_t *letter_create()
{
    letter_t *letter = malloc(sizeof(letter));
    letter->mail_from = malloc(sizeof(char) * 255);
    letter->rcpt_to = malloc(sizeof(char) * 255);
    letter->body = malloc(sizeof(char) * 1024 * 1024 * 2);
    letter->file = NULL;
    return letter;
}

void letter_free(letter_t *letter)
{
    free(letter->mail_from);
    free(letter->rcpt_to);
    free(letter->body);
    free(letter);
}
