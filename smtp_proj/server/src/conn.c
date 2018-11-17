#include "../include/conn.h"
#include "../include/pool.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#define QUEUE_LEN 100

static char* def_ip = "127.0.0.1";
static int def_port = 25;

int accept_conn(struct conn_opts *options) {
    const char* ip_addr = strlen(options->ip) == 0 ? def_ip : options->ip;
    const int port = options->port == 0 ? def_port : options->port;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    uint32_t addr_size = sizeof(client_addr);

    int listener;
    fd_set master;
    fd_set read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port);

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        // TODO: Логгирование ошибки
        return -1;
    }

    int reuse_addr = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != 0)
    {
        // TODO: Логгирование ошибки
        return -1;
    }

    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0)
    {
        // TODO: Логгирование ошибки
        goto DESTRUCT;
    }

    if (listen(listener, QUEUE_LEN) != 0)
    {
        // TODO: Логгирование ошибки
        goto DESTRUCT;
    }

    FD_SET(listener, &master);

    for (;;)
    {
        read_fds = master;
        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) == -1)
        {
            // TODO: Логгирование ошибки
            goto DESTRUCT;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, &read_fds))
            {
                if (i == listener)
                {
                    int new;
                    if ((new = accept(listener, (struct sockaddr *)&client_addr, &addr_size)) < 0)
                    {
                        // TODO: Логгирование ошибки
                        goto DESTRUCT;
                    };
                    FD_SET(new, &read_fds);
                }
            } else {
                char* buf = (char*) malloc(512);
                if (recv(i, &buf, 512, 0) < 0)
                {
                    close(i);
                    FD_CLR(i, &read_fds);
                }
                free(buf);
            }
    }

    return 0;

    DESTRUCT: close(listener);
    return -1;
}