#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

char *str_replace(char *str, char *orig, char *rep);
char **str_split(char *a_str, const char a_delim);

#endif