#ifndef _MAP_H_
#define _MAP_H_

#include <resolv.h>

struct mail_domain_dscrptr {
    char *domain;
    struct sockaddr_in domain_mail_server;
    int socket_fd;
};

struct mail_process_dscrptr {
    int domains_count;
    pid_t pid;
    void* shmem;
};


#endif // _SCHEDULER_H_