#ifndef _MX_UTILS_H_
#define _MX_UTILS_H_

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include "client_types.h"

#define N 4096

char *get_domain_mx_server_name(char *domain_name);
struct sockaddr_in get_domain_server_info(char *domain_name);

#endif