#include "protocol.h"

#include <stdlib.h>
#include <string.h>

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
    for (int i = 0; i < CONN_SIZE; i++)
    {
        conn_t *conn;
        if ((conn = connections[i]) == NULL)
            continue;
        if (conn->recv_buf[0] != '\0')
            printf("%s\n", conn->recv_buf);
        strcpy(conn->send_buf, conn->recv_buf);
        memset(conn->recv_buf, 0, sizeof(char) * RECV_BUF_SIZE);
    }
    return 0;
}