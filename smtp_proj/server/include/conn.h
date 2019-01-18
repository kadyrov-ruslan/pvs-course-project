#ifndef _CONN_H_
#define _CONN_H_

#include <netinet/in.h>
#include "server.h"
#include "letter.h"

extern int worker_count;
extern pid_t *worker_pids;

// Размер буфера приёма данных
#define SEND_BUF_SIZE 1024
// Размер буфера отправки данных
#define RECV_BUF_SIZE 1024

typedef struct
{
    const short sin_family;
    const struct sockaddr_in ipv4;
    const struct sockaddr_in6 ipv6;
    char send_buf[SEND_BUF_SIZE];
    char recv_buf[RECV_BUF_SIZE];
    letter_t *letter;
} conn_opts_t;

int accept_conn(const server_opts_t *opts);

/* Закрытие всех соединений с кодом 421 */
int close_conns();

/* Перевод сокета в неблокирующий режим */
int socket_nonblock(int socket_fd);

/* Создание процессов */
int prefork();

/* Обработка сигналов */
void handle_signal(int signal);

/* Обработка сигналов */
void handle_signal_master(int signal);

#endif // _CONN_H_