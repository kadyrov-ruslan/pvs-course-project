#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>

#include "../../common/include/map.h"
#include "../include/client_types.h"

int run_client();
int get_out_mail_domains(char **domains);
char *str_replace(char *str, char *orig, char *rep);
char **str_split(char *a_str, const char a_delim);


int get_output_mails(struct MapItem *items);
int count_dir_entries(const char *dirname);
char *get_domain_mx_server(char *domain_name);

void get_domain_server_info(char *domain_name);

#endif // _SCHEDULER_H_