#include "maildir.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int maildir_init()
{
    ensure_dir(opts.maildir);

    maildir_base[USR_LOCAL] = malloc(strlen(opts.maildir) + strlen("local") + 2);
    sprintf(maildir_base[USR_LOCAL], "%s/%s", opts.maildir, "local");
    if (ensure_dir(maildir_base[USR_LOCAL]) < 0)
        return -1;

    maildir_base[USR_REMOTE] = malloc(strlen(opts.maildir) + strlen("remote") + 2);
    sprintf(maildir_base[USR_REMOTE], "%s/%s", opts.maildir, "remote");
    if (ensure_dir(maildir_base[USR_REMOTE]) < 0)
        return -1;

    return 0;
}

int maildir_ensure_user(const char* username, user_type type)
{
    char *userdir = malloc(strlen(maildir_base[type]) + strlen(username) + 2);
    sprintf(userdir, "%s/%s", maildir_base[type], username);
    ensure_dir(userdir);

    char *newdir = malloc(strlen(userdir) + strlen("new") + 2);
    sprintf(newdir, "%s/%s", userdir, "new");
    ensure_dir(newdir);

    char *curdir = malloc(strlen(userdir) + strlen("cur") + 2);
    sprintf(curdir, "%s/%s", userdir, "cur");
    ensure_dir(curdir);

    char *tmpdir = malloc(strlen(userdir) + strlen("tmp") + 2);
    sprintf(tmpdir, "%s/%s", userdir, "tmp");
    ensure_dir(tmpdir);

    free(newdir);
    free(curdir);
    free(tmpdir);
    free(userdir);

    return 0;
}

int maildir_get_fname(const char* username, const char *domain, const char **fname)
{

    char *userdir = malloc(strlen(maildir_base[USR_LOCAL]) + strlen(username) + 2);
    sprintf(userdir, "%s/%s", maildir_base[USR_LOCAL], username);

    char *newdir = malloc(strlen(userdir) + strlen("new") + 2);
    sprintf(newdir, "%s/%s", userdir, "new");

    struct timeval tv;
    gettimeofday(&tv, NULL);
    char* filename = malloc(sizeof(char) * 255);

    pid_t pid = getpid();
    int timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    sprintf(filename, "%d.%d.%s.mbox", pid, timestamp, domain);

    char *fpath = malloc(strlen(newdir) + strlen(filename) + 2);
    sprintf(fpath, "%s/%s", newdir, filename);

    *fname = fpath;
    return 0;
}

int ensure_dir(const char* dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1)
        if (mkdir(dir, 0755) == -1)
            return -1;
    return 0;
}