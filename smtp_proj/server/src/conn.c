#include "conn.h"
#include "pool.h"

#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define QUEUE_LEN 100

static char def_ip[32] = "127.0.0.1";
static uint16_t def_port = 25;

int accept_conn(struct conn_opts *options) {
    char* ip_addr = strlen(options->ip) == 0 ? def_ip : options->ip;
    uint16_t port = options->port == 0 ? def_port : options->port;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        // TODO: Логгирование ошибки
        return -1;
    }

    int reuse_addr = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != 0)
    {
        // TODO: Логгирование ошибки
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr)) != 0)
    {
        // TODO: Логгирование ошибки
        close(sock);
        return -1;
    }

    if (listen(sock, QUEUE_LEN) != 0)
    {
        // TODO: Логгирование ошибки
        close(sock);
        return -1;
    }

    return 0;
}