#ifndef _CONN_H_
#define _CONN_H_

#include <server.h> 
#include <netinet/in.h>

extern int worker_count;
extern pid_t *worker_pids;

typedef struct
{
    const short sin_family;
    const struct sockaddr_in ipv4;
    const struct sockaddr_in6 ipv6;
    char send_buf[1024];
    char recv_buf[1024];
} conn_opts_t;

int accept_conn(const server_opts_t *opts);

/* Перевод сокета в неблокирующий режим */
int socket_nonblock(int socket_fd);

/* Создание процессов */
int prefork();

/* Обработка сигналов */
void handle_signal(int signal);

/* Обработка сигналов */
void handle_signal_master(int signal);

#endif // _CONN_H_