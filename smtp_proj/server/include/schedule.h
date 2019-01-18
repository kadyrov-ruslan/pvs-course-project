#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include <stdint.h>

struct accept_opts {
    const char* ip;
    int port;
};

int accept_conn(struct accept_opts *options);

#endif // _SCHEDULE_H_