#include "protocol.h"

#include "pattern.h"

patterns_t patterns;

int protocol_init()
{
    if (pattern_init() != 0)
        return -1;
    return 0;
}

int protocol_update()
{
    return 0;
}