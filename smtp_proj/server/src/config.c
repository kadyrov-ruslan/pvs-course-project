#include "config.h"

int server_opts_init(server_opts_t *opts, const config_t *config)
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
    if (config_setting_lookup_string(system, "domain", &opts->domain) != CONFIG_TRUE)
        return -70;
    if (config_setting_lookup_string(system, "maildir", &opts->maildir) != CONFIG_TRUE)
        return -80;

    return 0;
}

int log_opts_init(log_opts_t *opts, const config_t *config)
{
    config_setting_t *log = config_lookup(config, "log");

    if (log == NULL)
        return -10;
    if (config_setting_lookup_string(log, "log_file", &opts->path) != CONFIG_TRUE)
        return -20;

    return 0;
}

char *server_opts_err(int code)
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
        case -70: return "Invalid {system.domain} value in config";
        case -80: return "Invalid {system.maildir} value in config";
        default: return "Unrecognized error";
    }
}

char *log_opts_err(int code)
{
    switch (code)
    {
        case 0: return "Normal execution";
        case -10: return "No {log} section in config";
        case -20: return "No {log.log_file} string value in config";
        default: return "Unrecognized error";
    }
}
