#ifndef _CONN_H_
#define _CONN_H_

#include <stdint.h>

struct conn_opts {
    char ip[32];
    uint16_t port;
};

int accept_conn(struct conn_opts *options);

#endif // _CONN_H_