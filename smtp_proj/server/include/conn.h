#ifndef _CONN_H_
#define _CONN_H_

#include <server.h> 
#include <netinet/in.h>

typedef struct conn_opts_t {
    const struct sockaddr_in ipv4;
    const struct sockaddr_in6 ipv6;
} conn_opts_t;

int accept_conn(const server_opts_t *opts);

#endif // _CONN_H_