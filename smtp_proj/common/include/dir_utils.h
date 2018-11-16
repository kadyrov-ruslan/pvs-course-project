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

int count_dir_entries(const char *dirname);

#endif