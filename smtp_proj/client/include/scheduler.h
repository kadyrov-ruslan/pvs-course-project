#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

//#include "client_types.h"
#include "../../common/include/map.h"
#include <stdbool.h>

int run_client();
void check_output_mails();
int countEntriesInDir(const char *dirname);
int isDirectory(const char *path);

#endif // _SCHEDULER_H_