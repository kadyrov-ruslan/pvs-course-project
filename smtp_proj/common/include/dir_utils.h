#ifndef _DIR_UTILS_H_
#define _DIR_UTILS_H_

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
#include "string_utils.h"

int count_dir_entries(const char *dirname);
char *get_user_new_dir_full_path(const char *mail_dir, char *user_dir_name);
char *get_domain_name_from_email_full_path(char *email_path);

#endif