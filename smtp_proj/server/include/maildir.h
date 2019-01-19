#ifndef _MAILDIR_H_
#define _MAILDIR_H_

#include "server.h"

extern server_opts_t opts;

typedef enum
{
    USR_LOCAL,
    USR_REMOTE
} user_type;

char *maildir_base[USR_REMOTE + 1];

int maildir_init();

int maildir_ensure_user(const char *username, user_type type);

int maildir_get_fname(const char* username, const char *domain, const char **fname);

int ensure_dir(const char *dir);

#endif // _MAILDIR_H_
