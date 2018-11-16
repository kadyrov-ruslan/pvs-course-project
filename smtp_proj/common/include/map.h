#ifndef _MAP_H_
#define _MAP_H_

struct MapItem {
    char *full_domain_dir;
    char *domain;
    //вместо массива fd 
    //открываем письма по очереди а не сразу
    int *values;
    int values_count;
};

struct mail_domain_dscrptr {
    char *domain;
    struct sockaddr_in domain_mail_server;
    int socket_fd;
};


#endif // _SCHEDULER_H_