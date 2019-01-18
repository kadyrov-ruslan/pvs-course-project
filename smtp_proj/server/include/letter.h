#ifndef _LETTER_H_
#define _LETTER_H_

#include <stdio.h>

typedef struct
{
    char mail_from[255]; // Отправитель
    char* rcpt_to[255]; // Получатели
    char* body; // Тело письма
    FILE* file; // Файл для записи
} letter_t;

#endif // _LETTER_H_