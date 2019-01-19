#include "conn.h"

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include "log.h"
#include "protocol.h"

#define QUEUE_LEN 1

const char* def_ip = "127.0.0.1";
const int def_port = 25;

pid_t pid;
int master;
struct sigaction sa;
extern int worker_count;
extern pid_t log_pid, *worker_pids;

int listener, new_fd, max_fd;
fd_set active_read_fds, active_write_fds, read_fds, write_fds;

size_t nbytes;
char buf[1024];
struct timeval timeout;
struct sockaddr_in client_addr;
uint32_t addr_size = sizeof(client_addr);

int conn_accept(const server_opts_t *opts) {
    const char* ip_addr = strlen(opts->ip) == 0 ? def_ip : opts->ip;
    const int port = opts->port == 0 ? def_port : opts->port;
    struct sockaddr_in server_addr;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port);

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Can't create socket");
        goto DESTRUCT;
    }

    if (socket_nonblock(listener) < 0)
        goto DESTRUCT;

    int reuse_addr = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != 0)
    {
        fprintf(stderr, "Can't set SO_REUSEADDR to socket");
        goto DESTRUCT;
    }

    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0)
    {
        fprintf(stderr, "Can't bind to %s:%d\n", opts->ip, opts->port);
        goto DESTRUCT;
    }

    if (listen(listener, QUEUE_LEN) != 0)
    {
        fprintf(stderr, "Can't listen on %s:%d\n", opts->ip, opts->port);
        goto DESTRUCT;
    }

    log_i("SMTP server listen on %s:%d", opts->ip, opts->port);

    max_fd = listener;
    FD_ZERO(&active_read_fds);
    FD_ZERO(&active_write_fds);
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(listener, &active_read_fds);

    for (int i = 0; i < CONN_SIZE; ++i)
        connections[i] = NULL;

    if (prefork() < 0)
    {
        fprintf(stderr, "Prefork error\n");
        goto DESTRUCT;
    }

    while (1)
    {
        conn_update();
        protocol_update();
    }

    return 0;

DESTRUCT:
    close(listener);
    return -1;
}

int conn_update()
{
    read_fds = active_read_fds;
    write_fds = active_write_fds;
    if (select(max_fd + 1, &read_fds, &write_fds, NULL, &timeout) == -1)
    {
        fprintf(stderr, "Can't execute select syscall\n");
        return -1;
    }

    for (int i = 0; i <= max_fd; ++i)
    {
        if (FD_ISSET(i, &read_fds))
        {
            if (i == listener)
            {
                if ((new_fd = accept(listener, (struct sockaddr *)&client_addr, &addr_size)) < 0)
                {
                    log_e("(%d) %s", pid, "Can't accept new connection");
                }
                else
                {
                    log_i("(%d) Accept connection from %s:%d", pid, inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
                    socket_nonblock(new_fd);

                    connections[new_fd] = malloc(sizeof(conn_t));
                    watches[new_fd] = malloc(sizeof(stopwatch_t));
                    connections[new_fd]->state = SERVER_ST_INIT;
                    connections[new_fd]->watch = watches[new_fd];
                    stopwatch_start(watches[new_fd]);

                    if (new_fd > max_fd)
                        max_fd = new_fd;
                    FD_SET(new_fd, &active_read_fds);
                }
            }
            else {
                if ((nbytes = recv(i, &buf, 1024, 0)) <= 0)
                {
                    close(i);
                    FD_CLR(i, &read_fds);
                }
                conn_t *conn = connections[i];
                strcpy(conn->recv_buf, buf);
                memset(buf, 0, strlen(buf));
            }
        }
        else if (FD_ISSET(i, &write_fds))
        {
            conn_t *conn;
            if ((conn = connections[i]) == NULL)
                continue;
            if (send(i, conn->send_buf, strlen(conn->send_buf), 0) != -1)
            {
                FD_CLR(i, &active_write_fds);
                FD_SET(i, &active_read_fds);
            }
            else if (errno != EAGAIN || errno != EWOULDBLOCK)
            {
                log_w("IS_SET: %s", "Send to socket failure");
            }
        }
    }

    for (int i = 0; i <= max_fd; ++i)
    {
        conn_t *conn;
        if ((conn = connections[i]) == NULL)
            continue;
        if (conn->send_buf[0] != '\0')
        {
            nbytes = send(i, conn->send_buf, strlen(conn->send_buf), 0);
            if (nbytes == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    FD_CLR(i, &active_read_fds);
                    FD_SET(i, &active_write_fds);
                }
                else
                    log_w("%s", "Send to socket failure");
            }

            memset(conn->send_buf, 0, sizeof(char) * SEND_BUF_SIZE);
        }
        if (conn->state == SERVER_ST_DISCONNECTED)
        {
            free(connections[i]);
            free(watches[i]);
            FD_CLR(i, &active_read_fds);
            FD_CLR(i, &active_write_fds);
            close(i);
        }
    }

    return 0;
}

int conn_destroy()
{
    for (int i = 0; i <= max_fd; i++)
        if (FD_ISSET(i, &read_fds) || FD_ISSET(i, &write_fds))
        {
            if (send(i, response_421, strlen(response_421), 0) == -1)
                log_e("%s", "Can't close connection");
            close(i);
        }

    return 0;
}

int socket_nonblock(int socket_fd) {
    int flags;
    if ((flags = fcntl(socket_fd, F_GETFL, NULL)) < 0)
        return -1;

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) < 0)
        return -1;
    return 0;
}

int prefork()
{
    master = 1;
    struct sigaction sa = {0};
    for (pid_t i = 0; i < worker_count; ++i)
    {
        if ((worker_pids[i] = fork()) == 0)
        {
            master = 0;
            sa.sa_handler = &handle_signal;
            break;
        }
        else if (worker_pids[i] == -1)
        {
            fprintf(stderr, "Can't fork worker process\n");
            return -1;
        }
    }

    if (master == 1)
        sa.sa_handler = &handle_signal_master;

    sigfillset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        fprintf(stderr, "Can't handle SIGINT\n");
        handle_signal(SIGINT);
    }

    pid = getpid();
    return 0;
}

void handle_signal(int signal)
{
    switch (signal)
    {
    case SIGINT:
        conn_destroy();
        exit(0);
    }
}

void handle_signal_master(int signal)
{
    switch (signal)
    {
    case SIGINT:
        kill(log_pid, SIGINT);
        for (pid_t i = 0; i < worker_count; ++i)
            kill(worker_pids[i], SIGINT);
        conn_destroy();
        printf("SMTP server exiting\n");
        exit(0);
    }
}
