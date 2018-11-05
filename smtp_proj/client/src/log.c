#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

log_level cur_lvl;

int start_logger(const char *log_filename_base)
{
    return 0;
}

int stop_logger(void)
{
    int result = 0;
    return result;
}

int send_log_message(log_level log_lvl, char *message)
{
    int result = 0;
    return result;
}
