#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <libconfig.h>

#include "log.h"

#define USAGE "Usage: smtp_server <config file>\n"
#define PROCESS_DEFAULT 4 

int server_opts_init(server_options_t *opts, const config_t *config);

int log_opts_init(log_options_t *opts, const config_t *config);

char *server_opts_error(int code);

char *log_opts_error(int code);

int main(int argc, char **argv)
{
    int err;
    config_t config;
    config_init(&config);
    server_options_t opts;
    log_options_t log_opts;

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

    if (fork() == 0)
        logger_start(&log_opts);
    else
    {
        log_message(INFO, "Server started");
        sleep(5);
    }

    config_destroy(&config);
    return EXIT_SUCCESS;

DESTRUCT:
    config_destroy(&config);
    return EXIT_FAILURE;
}

int server_opts_init(server_options_t *opts, const config_t *config)
{
    config_setting_t *system = config_lookup(config, "system");

    if (system == NULL)
        return -10;
    if (config_setting_lookup_string(system, "bind_ip", &opts->ip) != CONFIG_TRUE)
        return -20;
    if (config_setting_lookup_int(system, "port", &opts->port) != CONFIG_TRUE)
        return -30;
    if (opts->port < 0 || opts->port > 65535)
        return -31;
    if (config_setting_lookup_string(system, "user", &opts->user) != CONFIG_TRUE)
        return -40;
    if (config_setting_lookup_string(system, "group", &opts->group) != CONFIG_TRUE)
        return -50;
    if (config_setting_lookup_int(system, "process_count", &opts->process_count) != CONFIG_TRUE)
        return -60;
    if (opts->process_count < 0 || opts->process_count > 1024)
        return -61;

    return 0;
}

int log_opts_init(log_options_t *opts, const config_t *config)
{
    config_setting_t *log = config_lookup(config, "log");

    if (log == NULL)
        return -10;
    if (config_setting_lookup_string(log, "log_file", &opts->path) != CONFIG_TRUE)
        return -20;

    return 0;
}

char *server_opts_error(int code)
{
    switch (code)
    {
        case 0: return "Normal execution";
        case -10: return "No {system} section in config";
        case -20: return "No {system.bind_ip} string value in config";
        case -30: return "No {system.port} integer value in config";
        case -31: return "Invalid {system.port} value in config";
        case -40: return "No {system.user} string value in config";
        case -50: return "No {system.group} string value in config";
        case -60: return "No {system.process_count} int value in config";
        case -61: return "Invalid {system.process_count} value in config";
        default: return "Unrecognized error";
    }
}

char *log_opts_error(int code)
{
    switch (code)
    {
        case 0: return "Normal execution";
        case -10: return "No {log} section in config";
        case -20: return "No {log.log_file} string value in config";
        default: return "Unrecognized error";
    }
}
