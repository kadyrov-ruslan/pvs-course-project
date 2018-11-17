#ifndef _CONN_H_
#define _CONN_H_

#include <stdint.h>

struct conn_opts {
    const char* ip;
    const char* user;
    const char* group;
    int port;
    int process_count;
};

int accept_conn(struct conn_opts *options);

#endif // _CONN_H_