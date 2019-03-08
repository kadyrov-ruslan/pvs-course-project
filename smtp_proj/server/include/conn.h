#ifndef _CONN_H_
#define _CONN_H_

#include <netinet/in.h>

#include "server.h"
#include "letter.h"
#include "smtp-fsm.h"
#include "stopwatch.h"

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
    char* send_buf;
    char* recv_buf;
    te_server_state state;
    te_server_state old_state;
    letter_t *letter;
    stopwatch_t *watch;
} conn_t;

conn_t *connections[CONN_SIZE];
stopwatch_t *watches[CONN_SIZE];

int conn_accept(const server_opts_t *opts);

/* Обновление соединений */
int conn_update();

/* Закрытие всех соединений с кодом 421 */
int conn_destroy();

/* Создание conn_t */
conn_t *conn_create();

/* Освобождение conn_t */
void conn_free(conn_t *conn);


/* Перевод сокета в неблокирующий режим */
int socket_nonblock(int socket_fd);

/* Создание процессов */
int prefork();

/* Обработка сигналов */
void handle_signal(int signal);

/* Обработка сигналов */
void handle_signal_master(int signal);

#endif // _CONN_H_