#ifndef _MAP_H_
#define _MAP_H_

#include <resolv.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 

struct mail_domain_dscrptr {
    char *domain;
    struct sockaddr_in domain_mail_server;
    int socket_fd;
};

struct mail_process_dscrptr {
    pid_t pid;
    int msg_queue_id;
    int domains_count;
    int mails_count;
    char *domains[60];
};

//Describes single domain mails, that have to be sent
struct domain_mails {
    char *domain;
    char *mails_paths[100];
    int mails_count;
};


#endif // _SCHEDULER_H_