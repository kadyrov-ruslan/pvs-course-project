#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <libconfig.h>

#include "server.h"
#include "log.h"

int server_opts_init(server_opts_t *opts, const config_t *config);

int log_opts_init(log_opts_t *opts, const config_t *config);

char *server_opts_err(int code);

char *log_opts_err(int code);

#endif // _CONFIG_H_