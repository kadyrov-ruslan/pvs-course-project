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
// Количество одновременных соединений
#define CONN_SIZE 1024

typedef struct
{
    const short sin_family;
    const struct sockaddr_in ipv4;
    const struct sockaddr_in6 ipv6;
    char send_buf[SEND_BUF_SIZE];
    char recv_buf[RECV_BUF_SIZE];
    letter_t *letter;
} conn_t;

conn_t *connections[CONN_SIZE];

int conn_accept(const server_opts_t *opts);

/* Обновление соединений */
int conn_update();

/* Закрытие всех соединений с кодом 421 */
int conn_destroy();

/* Перевод сокета в неблокирующий режим */
int socket_nonblock(int socket_fd);

/* Создание процессов */
int prefork();

/* Обработка сигналов */
void handle_signal(int signal);

/* Обработка сигналов */
void handle_signal_master(int signal);

#endif // _CONN_H_