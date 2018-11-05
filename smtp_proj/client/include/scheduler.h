#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "client_types.h"

#include <stdbool.h>
#include <netdb.h>
#include <sys/socket.h>


int run_client();
int check_output_mails();

#endif // _SCHEDULER_H_