#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "client_types.h"
#include <stdbool.h>

int run_client();
void check_output_mails();
int countEntriesInDir(const char *dirname);

#endif // _SCHEDULER_H_