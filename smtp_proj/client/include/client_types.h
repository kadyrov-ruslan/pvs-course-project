#ifndef _CLIENT_TYPES_H_
#define _CLIENT_TYPES_H_

#include "log.h"

#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

struct client_conf {
	int port;
	int proc_cnt;

	log_level log_lvl;
	const char *log_file;
	const char *mail_dir;
	//const char *queue_dir;
	const char *hostname;
};

extern struct client_conf conf;

#endif // _CLIENT_TYPES_H_

