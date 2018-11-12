#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <assert.h>

#include "../../common/include/map.h"
#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/client_types.h"

int run_client();
int get_out_mail_domains(char **domains);
int get_domains_diff(int new_domains_count, char **new_mail_domains, char **dif);

int get_output_mails(struct MapItem *items);
char *get_domain_mx_server(char *domain_name);

struct sockaddr_in get_domain_server_info(char *domain_name);

#endif // _SCHEDULER_H_