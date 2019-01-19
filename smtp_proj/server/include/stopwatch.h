#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

#include <sys/time.h>

typedef struct
{
    struct timeval tv1;
} stopwatch_t;

int stopwatch_start(stopwatch_t *watch);

/* Возвращает время в мс */
int stopwatch_watch(const stopwatch_t *watch);

#endif // _STOPWATCH_H_