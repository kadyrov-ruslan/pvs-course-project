#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "../../common/include/map.h"
#include <stdbool.h>

int run_client();
int get_output_mails(struct MapItem *items);
int countEntriesInDir(const char *dirname);
int isDirectory(const char *path);

void get_server_info(char *SMTP_Server_Host_Name);

#endif // _SCHEDULER_H_