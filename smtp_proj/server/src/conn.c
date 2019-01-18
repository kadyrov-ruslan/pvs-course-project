#include "conn.h"

#include <fcntl.h>
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

#define QUEUE_LEN 1

const char* def_ip = "127.0.0.1";
const int def_port = 25;

pid_t pid;
int master;
struct sigaction sa;
extern int worker_count;
extern pid_t log_pid, *worker_pids;

int listener, new_fd, max_fd;
fd_set active_read_fds, read_fds, write_fds;

char buf[1024];
size_t nbytes;

int accept_conn(const server_opts_t *opts) {
    const char* ip_addr = strlen(opts->ip) == 0 ? def_ip : opts->ip;
    const int port = opts->port == 0 ? def_port : opts->port;
    struct sockaddr_in server_addr;
    struct timeval timeout;

    struct sockaddr_in client_addr;
    uint32_t addr_size = sizeof(client_addr);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10;

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
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(listener, &active_read_fds);

    if (prefork() < 0)
    {
        fprintf(stderr, "Prefork error\n");
        goto DESTRUCT;
    }

    while (1)
    {
        read_fds = active_read_fds;
        if (select(max_fd + 1, &read_fds, NULL, NULL, &timeout) == -1)
        {
            fprintf(stderr, "Can't execute select syscall\n");
            goto DESTRUCT;
        }

        for (int i = 0; i <= max_fd; ++i)
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

                        if (new_fd > max_fd)
                            max_fd = new_fd;
                        FD_SET(new_fd, &read_fds);
                    }
                }
                else {
                    if ((nbytes = recv(i, &buf, 1024, 0)) <= 0)
                    {
                        close(i);
                        FD_CLR(i, &read_fds);
                    }
                    send(i, buf, nbytes, 0);
                }
            }
    }

    return 0;

DESTRUCT:
    close(listener);
    return -1;
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
        printf("SMTP server exiting\n");
        exit(0);
    }
}