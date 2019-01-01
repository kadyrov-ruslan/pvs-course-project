#ifndef _CLIENT_TYPES_H_
#define _CLIENT_TYPES_H_

#include "log.h"
#include "mail_domain.h"

#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

struct client_conf
{
	int port;
	int proc_cnt;

	log_level log_lvl;
	const char *logs_dir;
	const char *mail_dir;
	//const char *queue_dir;
	const char *hostname;

	struct MailDomain mail_domains[3]; 
};

extern struct client_conf conf;

#endif // _CLIENT_TYPES_H_
