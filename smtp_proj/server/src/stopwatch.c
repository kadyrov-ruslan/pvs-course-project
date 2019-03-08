#include "stopwatch.h"

#include <stdlib.h>

int stopwatch_start(stopwatch_t *watch)
{
    gettimeofday(&watch->tv1, NULL);
    return watch->tv1.tv_sec * 1000 + watch->tv1.tv_usec / 1000;
}

/* Возвращает время в мс */
int stopwatch_watch(const stopwatch_t *watch)
{
    struct timeval tv2, dtv;
    gettimeofday(&tv2, NULL);
    dtv.tv_sec= tv2.tv_sec -watch->tv1.tv_sec;
    dtv.tv_usec=tv2.tv_usec-watch->tv1.tv_usec;
    if(dtv.tv_usec < 0)
    {
        dtv.tv_sec--;
        dtv.tv_usec += 1000000;
    }
    return dtv.tv_sec * 1000 + dtv.tv_usec / 1000;
}