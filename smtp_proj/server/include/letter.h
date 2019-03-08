#ifndef _LETTER_H_
#define _LETTER_H_

#include <stdio.h>

typedef struct
{
    char *mail_from; // Отправитель
    char *rcpt_to; // Получатель
    char *rcpt_username; // Имя пользователя
    char *rcpt_domain; // Домен
    char* body; // Тело письма
    FILE* file; // Файл для записи
} letter_t;

letter_t *letter_create();

void letter_free(letter_t *letter);

#endif // _LETTER_H_