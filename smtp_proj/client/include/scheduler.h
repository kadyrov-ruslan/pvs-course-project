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

#include "../../common/include/map.h"
#include "../include/client_types.h"

int run_client();
int get_output_mails(struct MapItem *items);
int count_dir_entries(const char *dirname);

void get_domain_server_info(char *domain_name);

#endif // _SCHEDULER_H_