#include "server.h"

#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <libconfig.h>

#include "log.h"
#include "conn.h"
#include "config.h"
#include "maildir.h"
#include "protocol.h"

#define USAGE "Usage: smtp_server <config file>\n"

server_opts_t opts;
log_level cur_level;

int worker_count;
pid_t log_pid, *worker_pids = NULL;

int main(int argc, char **argv)
{
    int err;
    config_t config;
    log_opts_t log_opts;

    config_init(&config);

    if (argc != 2)
    {
        fprintf(stderr, USAGE);
        goto DESTRUCT;
    }

    if ((err = config_read_file(&config, argv[1])) == CONFIG_FALSE)
    {
        const char* error_file = config_error_file(&config);
        if (error_file == NULL)
            error_file = argv[1];
        fprintf(stderr, "%s:%d - %s\n", error_file, config_error_line(&config), config_error_text(&config));
        goto DESTRUCT;
    }

    if ((err = server_opts_init(&opts, &config)) != 0)
    {
        fprintf(stderr, "Parsing server options: %s\n", server_opts_err(err));
        goto DESTRUCT;
    }

    if ((err = log_opts_init(&log_opts, &config)) != 0)
    {
        fprintf(stderr, "Parsing log options: %s\n", server_opts_err(err));
        goto DESTRUCT;
    }

    cur_level = DEBUG;
    if ((err = set_id(opts.user, opts.group)) != 0)
    {
        fprintf(stderr, "%s\n", set_id_err(err));
        goto DESTRUCT;
    }

    if ((err = protocol_init()) != 0)
        goto DESTRUCT;

    if ((err = maildir_init()) != 0)
        goto DESTRUCT;

    switch (log_pid = fork())
    {
    case 0:
        logger_start(&log_opts);
    case -1:
        fprintf(stderr, "Can't fork log process\n");
        goto DESTRUCT;
    default:
        worker_count = opts.process_count;
        worker_pids = malloc(worker_count * sizeof(pid_t));

        log_i("%s", "SMTP server booted");
        if ((err = conn_accept(&opts)) != 0)
            goto DESTRUCT;
    }

    free(worker_pids);
    config_destroy(&config);
    return EXIT_SUCCESS;

DESTRUCT:
    if (errno != 0)
        fprintf(stderr, "%s\n", strerror(errno));

    kill(log_pid, SIGINT);
    if (worker_pids != NULL)
        for (pid_t i = 0; i < worker_count; ++i)
            kill(worker_pids[i], SIGINT);

    free(worker_pids);
    config_destroy(&config);
    return EXIT_FAILURE;
}

int set_id(const char *user, const char *group)
{
    struct passwd *pwd;
    if ((pwd = getpwnam(user)) == NULL)
        return -10;

    struct group *grp;
    if ((grp = getgrnam(group)) == NULL)
        return -20;

    if (setuid(pwd->pw_uid) == -1)
        return -11;
    if (setgid(grp->gr_gid) == -1)
        return -21;
    return 0;
}

char *set_id_err(int code)
{
    switch (code)
    {
        case 0: return "Normal execution";
        case -10: return "User does not exist";
        case -11: return "Can't setuid() to user";
        case -20: return "Group does not exist";
        case -21: return "Can't setuid() to group";
        default: return "Unrecognized error";
    }
}
