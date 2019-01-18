#include "server.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <libconfig.h>

#include "log.h"
#include "config.h"
#include "worker.h"

#define USAGE "Usage: smtp_server <config file>\n"

void handle_signal(int signal);

pid_t log_pid;
int worker_count;
pid_t *worker_pids = NULL;
log_level cur_level;

int main(int argc, char **argv)
{
    int err;
    config_t config;
    server_options_t opts;
    log_options_t log_opts;
    struct sigaction sa;

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
        fprintf(stderr, "Parsing server options: %s\n", server_opts_error(err));
        goto DESTRUCT;
    }

    if ((err = log_opts_init(&log_opts, &config)) != 0)
    {
        fprintf(stderr, "Parsing log options: %s\n", server_opts_error(err));
        goto DESTRUCT;
    }

    cur_level = DEBUG;

    switch ((log_pid = fork()))
    {
    case 0:
        logger_start(&log_opts);
    case -1:
        fprintf(stderr, "Can't fork log process\n");
        goto DESTRUCT;
    default:
        worker_pids = malloc(opts.process_count * sizeof(pid_t));
        for (pid_t i = 0; i < opts.process_count; ++i)
        {
            if ((worker_pids[i] = fork()) == 0)
                worker_run();
            else if (worker_pids[i] == -1)
            {
                fprintf(stderr, "Can't fork worker process\n");
                goto DESTRUCT;
            }
        }

        sa.sa_handler = &handle_signal;
        sigfillset(&sa.sa_mask);

        if (sigaction(SIGINT, &sa, NULL) == -1)
        {
            fprintf(stderr, "Can't handle SIGINT\n");
            handle_signal(SIGINT);
        }

        log_i("%s", "SMTP server started");
        sleep(5);
        handle_signal(SIGINT);
    }

    free(worker_pids);
    config_destroy(&config);
    return EXIT_SUCCESS;

DESTRUCT:
    free(worker_pids);
    config_destroy(&config);
    return EXIT_FAILURE;
}

void handle_signal(int signal)
{
    switch (signal)
    {
        case SIGINT:
        {
            kill(log_pid, SIGINT);
            for (pid_t i = 0; i < worker_count; ++i)
                kill(worker_pids[i], SIGINT);
            printf("SMTP server exiting\n");
            exit(0);
        }
    }
}
