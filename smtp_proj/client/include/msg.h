#ifndef _MSG_H_
#define _MSG_H_

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

#define INITIAL_SIZE 10
char *read_msg_file(char *email_path);

#endif